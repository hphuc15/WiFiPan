#include "WiFiPan_Internal.h"
#include "WiFiPan_Html.h"


#include "esp_ota_ops.h"
#include "esp_log.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"
#include "sys/param.h"
#include "netinet/in.h"

#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <string>

namespace WiFiPan
{
    namespace
    {
        constexpr const char *kTag    = "[WP][PORTAL]";
        constexpr const char *kTagDns = "[WP][DNS]";
        constexpr const char *kTagDhcp = "[WP][DHCP]";

        constexpr size_t kOtaRecvBufSize = 2048;
        constexpr size_t kOtaTailMargin  = 64;

        constexpr uint16_t kDnsPort           = 53;
        constexpr size_t   kDnsMaxPacketSize  = 512;  /* RFC 1035 §2.3.4 UDP limit */
        constexpr size_t   kDnsLabelMaxLen    = 63;
        constexpr size_t   kDnsNameMaxLen     = 255;
        constexpr uint32_t kDnsTtlDefault     = 60;
        constexpr size_t   kDnsHeaderSize     = 12;

        constexpr uint16_t kDnsTypeA    = 1;
        constexpr uint16_t kDnsTypeAny  = 255;
        constexpr uint16_t kDnsClassIn  = 1;

        constexpr uint16_t kDnsFlagQrMask     = 0x8000;
        constexpr uint16_t kDnsFlagOpcodeMask = 0x7800;
        constexpr uint16_t kDnsFlagAaMask     = 0x0400;
        constexpr uint16_t kDnsFlagRcodeMask  = 0x000F;

        constexpr uint16_t kDnsRcodeNoError = 0;

        constexpr uint16_t kDnsPtrOffset      = kDnsHeaderSize;
        constexpr uint16_t kDnsPtrCompression = 0xC000u | kDnsPtrOffset;

        inline bool DnsIsPtr(uint8_t b) { return (b & 0xC0) == 0xC0; }
        inline uint16_t DnsFlagGetQr(uint16_t f)     { return (f & kDnsFlagQrMask) >> 15; }
        inline uint16_t DnsFlagGetOpcode(uint16_t f) { return (f & kDnsFlagOpcodeMask) >> 11; }
        inline uint16_t DnsFlagSetQr(uint16_t f)     { return f | kDnsFlagQrMask; }
        inline uint16_t DnsFlagSetAa(uint16_t f)     { return f | kDnsFlagAaMask; }
        inline uint16_t DnsFlagSetRcode(uint16_t f, uint16_t r) { return (f & ~kDnsFlagRcodeMask) | (r & kDnsFlagRcodeMask); }

        enum class DnsStatus
        {
            Ok = 0,
            ErrSocket,
            ErrBind,
            ErrRecv,
            ErrSend,
            ErrBuild,
            ErrInvalidIp,
        };

        const char *DnsStatusToStr(DnsStatus status)
        {
            switch (status) {
            case DnsStatus::Ok:          return "Ok";
            case DnsStatus::ErrSocket:   return "ErrSocket";
            case DnsStatus::ErrBind:     return "ErrBind";
            case DnsStatus::ErrRecv:     return "ErrRecv";
            case DnsStatus::ErrSend:     return "ErrSend";
            case DnsStatus::ErrBuild:    return "ErrBuild";
            case DnsStatus::ErrInvalidIp:return "ErrInvalidIp";
            default:                     return "Unknown";
            }
        }

        struct __attribute__((packed)) DnsHeader
        {
            uint16_t id;
            uint16_t flags;
            uint16_t qd_count;
            uint16_t an_count;
            uint16_t ns_count;
            uint16_t ar_count;
        };

        struct __attribute__((packed)) DnsAnswer
        {
            uint16_t name_ptr;
            uint16_t type;
            uint16_t rr_class;
            uint32_t ttl;
            uint16_t rdlength;
            uint32_t rdata;
        };

        struct DnsServerState
        {
            int      sockfd = -1;
            uint32_t redirect_ip = 0;
            uint16_t port = 0;
            uint8_t  buf[kDnsMaxPacketSize]{};
            bool     running = false;
        };

        /* Skip a QNAME, return offset after it, or 0 on error. */
        size_t DnsSkipQname(const uint8_t *buf, size_t len, size_t pos)
        {
            size_t limit = pos + kDnsNameMaxLen + 1;
            while (pos < len && pos < limit) {
                uint8_t b = buf[pos];
                if (b == 0x00) {
                    return pos + 1;
                }
                if (DnsIsPtr(b)) {
                    return pos + 2;
                }
                if (b > kDnsLabelMaxLen) {
                    ESP_LOGE(kTagDns, "label length %u exceeds max %u", b, (unsigned)kDnsLabelMaxLen);
                    return 0;
                }
                pos += 1 + static_cast<size_t>(b);
            }
            ESP_LOGE(kTagDns, "QNAME not terminated within packet bounds");
            return 0;
        }

        const uint8_t *DnsParseQuery(const uint8_t *buf, size_t len, DnsHeader *out_hdr, size_t *out_qname_end)
        {
            if (!buf || len < kDnsHeaderSize) {
                ESP_LOGE(kTagDns, "packet too short (%zu bytes)", len);
                return nullptr;
            }

            DnsHeader hdr;
            std::memcpy(&hdr, buf, kDnsHeaderSize);
            hdr.id       = ntohs(hdr.id);
            hdr.flags    = ntohs(hdr.flags);
            hdr.qd_count = ntohs(hdr.qd_count);
            hdr.an_count = ntohs(hdr.an_count);
            hdr.ns_count = ntohs(hdr.ns_count);
            hdr.ar_count = ntohs(hdr.ar_count);

            if (DnsFlagGetQr(hdr.flags)) {
                return nullptr; /* response packet, ignore */
            }
            if (DnsFlagGetOpcode(hdr.flags) != 0) {
                ESP_LOGE(kTagDns, "unsupported OPCODE %u", DnsFlagGetOpcode(hdr.flags));
                return nullptr;
            }
            if (hdr.qd_count == 0) {
                ESP_LOGE(kTagDns, "no questions in query");
                return nullptr;
            }

            size_t pos = DnsSkipQname(buf, len, kDnsHeaderSize);
            if (pos == 0) {
                return nullptr;
            }
            if (pos + 4 > len) {
                ESP_LOGE(kTagDns, "packet too short for QTYPE/QCLASS");
                return nullptr;
            }
            pos += 4;

            if (out_hdr)       *out_hdr = hdr;
            if (out_qname_end) *out_qname_end = pos;
            return buf + pos;
        }

        DnsStatus DnsBuildResponse(const uint8_t *query_buf, size_t qname_end, uint32_t redirect_ip,
                                    uint8_t *out_buf, size_t *out_len)
        {
            if (!query_buf || !out_buf || !out_len) {
                return DnsStatus::ErrBuild;
            }
            if (qname_end < kDnsHeaderSize + 4) {
                ESP_LOGE(kTagDns, "qname_end=%zu looks wrong", qname_end);
                return DnsStatus::ErrBuild;
            }

            uint16_t qtype;
            std::memcpy(&qtype, query_buf + qname_end - 4, sizeof(uint16_t));
            qtype = ntohs(qtype);
            const bool answer_with_a = (qtype == kDnsTypeA || qtype == kDnsTypeAny);

            size_t question_len = qname_end - kDnsHeaderSize;
            size_t resp_len = kDnsHeaderSize + question_len + (answer_with_a ? sizeof(DnsAnswer) : 0);
            if (resp_len > kDnsMaxPacketSize) {
                ESP_LOGE(kTagDns, "response would exceed max packet size (%zu)", resp_len);
                return DnsStatus::ErrBuild;
            }

            const auto *req_hdr = reinterpret_cast<const DnsHeader *>(query_buf);
            uint16_t flags = ntohs(req_hdr->flags);
            flags = DnsFlagSetQr(flags);
            flags = DnsFlagSetAa(flags);
            flags = DnsFlagSetRcode(flags, kDnsRcodeNoError);

            DnsHeader resp_hdr;
            resp_hdr.id       = req_hdr->id; /* keep wire order */
            resp_hdr.flags    = htons(flags);
            resp_hdr.qd_count = htons(1);
            resp_hdr.an_count = htons(answer_with_a ? 1u : 0u);
            resp_hdr.ns_count = 0;
            resp_hdr.ar_count = 0;

            uint8_t *p = out_buf;
            std::memcpy(p, &resp_hdr, kDnsHeaderSize);
            p += kDnsHeaderSize;

            std::memcpy(p, query_buf + kDnsHeaderSize, question_len);
            p += question_len;

            if (answer_with_a) {
                DnsAnswer ans;
                ans.name_ptr = htons(kDnsPtrCompression);
                ans.type     = htons(kDnsTypeA);
                ans.rr_class = htons(kDnsClassIn);
                ans.ttl      = htonl(kDnsTtlDefault);
                ans.rdlength = htons(4u);
                ans.rdata    = redirect_ip; /* already NBO */
                std::memcpy(p, &ans, sizeof(DnsAnswer));
                p += sizeof(DnsAnswer);
            }

            *out_len = static_cast<size_t>(p - out_buf);
            return DnsStatus::Ok;
        }

        DnsStatus DnsInit(DnsServerState &srv, const char *redirect_ip, uint16_t port)
        {
            srv = DnsServerState{};
            srv.sockfd  = -1;
            srv.port    = port;
            srv.running = false;

            struct in_addr addr;
            if (inet_pton(AF_INET, redirect_ip, &addr) != 1) {
                ESP_LOGE(kTagDns, "inet_pton failed for \"%s\"", redirect_ip);
                return DnsStatus::ErrInvalidIp;
            }
            srv.redirect_ip = addr.s_addr;
            ESP_LOGI(kTagDns, "init: redirect=%s port=%u", redirect_ip, port);
            return DnsStatus::Ok;
        }

        DnsStatus DnsStart(DnsServerState &srv)
        {
            srv.sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (srv.sockfd < 0) {
                ESP_LOGE(kTagDns, "socket() failed: %d", errno);
                return DnsStatus::ErrSocket;
            }

            int yes = 1;
            setsockopt(srv.sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

            int fl = fcntl(srv.sockfd, F_GETFL, 0);
            fcntl(srv.sockfd, F_SETFL, fl | O_NONBLOCK);

            struct sockaddr_in addr {};
            addr.sin_family      = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port        = htons(srv.port);

            if (bind(srv.sockfd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
                ESP_LOGE(kTagDns, "bind() failed on port %u: %d", srv.port, errno);
                close(srv.sockfd);
                srv.sockfd = -1;
                return DnsStatus::ErrBind;
            }

            srv.running = true;
            ESP_LOGI(kTagDns, "listening on UDP port %u", srv.port);
            return DnsStatus::Ok;
        }

        DnsStatus DnsPoll(DnsServerState &srv)
        {
            if (srv.sockfd < 0) {
                return DnsStatus::ErrRecv;
            }

            struct sockaddr_in client {};
            socklen_t client_len = sizeof(client);

            ssize_t recv_len = recvfrom(srv.sockfd, srv.buf, kDnsMaxPacketSize, 0,
                                         reinterpret_cast<struct sockaddr *>(&client), &client_len);
            if (recv_len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return DnsStatus::Ok;
                }
                ESP_LOGE(kTagDns, "recvfrom() error: %d", errno);
                return DnsStatus::ErrRecv;
            }

            DnsHeader hdr;
            size_t qname_end;
            if (!DnsParseQuery(srv.buf, static_cast<size_t>(recv_len), &hdr, &qname_end)) {
                return DnsStatus::Ok; /* bad/unsupported packet - discard silently */
            }

            uint8_t resp[kDnsMaxPacketSize];
            size_t  resp_len = 0;
            DnsStatus status = DnsBuildResponse(srv.buf, qname_end, srv.redirect_ip, resp, &resp_len);
            if (status != DnsStatus::Ok) {
                ESP_LOGE(kTagDns, "DnsBuildResponse: %s", DnsStatusToStr(status));
                return status;
            }

            ssize_t sent = sendto(srv.sockfd, resp, resp_len, 0,
                                   reinterpret_cast<struct sockaddr *>(&client), client_len);
            if (sent < 0) {
                ESP_LOGE(kTagDns, "sendto() error: %d", errno);
                return DnsStatus::ErrSend;
            }
            return DnsStatus::Ok;
        }

        void DnsStop(DnsServerState &srv)
        {
            if (srv.sockfd >= 0) {
                close(srv.sockfd);
                srv.sockfd = -1;
            }
            srv.running = false;
            ESP_LOGI(kTagDns, "stopped");
        }

        struct DnsHandle
        {
            DnsServerState dns;
            TaskHandle_t   task = nullptr;
        };

        void DnsTaskEntry(void *arg)
        {
            auto *dns = static_cast<DnsServerState *>(arg);
            ESP_LOGI(kTagDns, "Task started on port %u", dns->port);
            while (true) {
                DnsStatus err = DnsPoll(*dns);
                if (err != DnsStatus::Ok) {
                    ESP_LOGE(kTagDns, "Poll error: %s", DnsStatusToStr(err));
                }
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

        /* ============================================================
         * URL / body parsing helpers
         * ============================================================ */

        void UrlDecode(char *dst, const char *src, size_t dst_size)
        {
            char *end = dst + dst_size - 1;
            while (*src && dst < end) {
                if (*src == '+') {
                    *dst++ = ' '; src++;
                } else if (*src == '%' && src[1] && src[2]) {
                    char hex[3] = {src[1], src[2], '\0'};
                    *dst++ = static_cast<char>(std::strtol(hex, nullptr, 16));
                    src += 3;
                } else {
                    *dst++ = *src++;
                }
            }
            *dst = '\0';
        }

        /* Returns true if the ssid field was present and non-empty. */
        bool ParsePost(Manager &wp, char *body)
        {
            wifi_sta_config_t sta = wp.StaConfig();
            bool sta_touched = false;
            bool ssid_set    = false;

            char *sp_pair, *sp_kv;
            char *pair = strtok_r(body, "&", &sp_pair);
            while (pair) {
                char *key = strtok_r(pair, "=", &sp_kv);
                char *val = strtok_r(nullptr, "=", &sp_kv);
                if (key) {
                    char dk[kFieldLen] = {0};
                    char dv[kFieldLen] = {0};
                    UrlDecode(dk, key, sizeof(dk));
                    UrlDecode(dv, val ? val : "", sizeof(dv));

                    if (std::strcmp(dk, "ssid") == 0) {
                        std::strncpy(reinterpret_cast<char *>(sta.ssid), dv, sizeof(sta.ssid) - 1);
                        sta_touched = true;
                        ssid_set = (dv[0] != '\0');
                    } else if (std::strcmp(dk, "password") == 0) {
                        std::strncpy(reinterpret_cast<char *>(sta.password), dv, sizeof(sta.password) - 1);
                        sta.threshold.authmode = WIFI_AUTH_OPEN;
                        sta_touched = true;
                    } else {
                        wp.page().SetParamValue(dk, dv);
                    }
                }
                pair = strtok_r(nullptr, "&", &sp_pair);
            }

            if (sta_touched) {
                wp.SetStaConfig(sta);
            }
            return ssid_set;
        }

        esp_err_t RecvBody(httpd_req_t *req, char *buf, size_t buf_size)
        {
            if (req->content_len == 0) {
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
                return ESP_FAIL;
            }
            size_t to_read = MIN(req->content_len, buf_size - 1);
            int n = httpd_req_recv(req, buf, to_read);
            if (n <= 0) {
                if (n == HTTPD_SOCK_ERR_TIMEOUT) httpd_resp_send_408(req);
                return ESP_FAIL;
            }
            buf[n] = '\0';
            return ESP_OK;
        }

        /* Builds "window.wifiscandata=[...]" by running a blocking WiFi scan. */
        std::string BuildScanInjectScript(uint16_t max_count)
        {
            uint16_t max = max_count ? max_count : kScanDefaultMax;
            auto *ap_buf = static_cast<wifi_ap_record_t *>(std::calloc(max, sizeof(wifi_ap_record_t)));
            uint16_t fetched = 0;

            if (ap_buf) {
                wifi_scan_config_t cfg{};
                cfg.show_hidden = false;
                cfg.scan_type   = WIFI_SCAN_TYPE_ACTIVE;
                if (esp_wifi_scan_start(&cfg, true) == ESP_OK) {
                    uint16_t found = 0;
                    esp_wifi_scan_get_ap_num(&found);
                    fetched = found < max ? found : max;
                    if (esp_wifi_scan_get_ap_records(&fetched, ap_buf) != ESP_OK) {
                        fetched = 0;
                    }
                }
            }

            std::string out = "window.wifiscandata=[";
            for (uint16_t i = 0; i < fetched; i++) {
                if (ap_buf[i].ssid[0] == '\0') continue;
                bool enc = (ap_buf[i].authmode != WIFI_AUTH_OPEN);

                std::string ssid_json;
                for (const uint8_t *s = ap_buf[i].ssid; *s; ++s) {
                    if (*s == '"' || *s == '\\') ssid_json += '\\';
                    ssid_json += static_cast<char>(*s);
                }

                if (i > 0) out += ',';
                out += "{\"ssid\":\"" + ssid_json + "\",\"rssi\":" +
                       std::to_string(ap_buf[i].rssi) + ",\"enc\":" + (enc ? "true" : "false") + "}";
            }
            out += "];";

            std::free(ap_buf);
            return out;
        }

        /* ============================================================
         * OTA helpers
         * ============================================================ */

        /* Returns true if the request is authorized. If wp->AdminToken() is empty,
        * auth is disabled (open) — logs a warning so it's visible at runtime. */
        bool CheckAdminToken(httpd_req_t *req, Manager *wp)
        {
            const char *expected = wp->AdminToken();
            if (expected[0] == '\0') {
                ESP_LOGW(kTag, "Admin token not set — /ota and /reset are UNAUTHENTICATED");
                return true;
            }

            char got[kAdminTokenLen] = {0};

            /* Try header first. */
            if (httpd_req_get_hdr_value_str(req, "X-Admin-Token", got, sizeof(got)) != ESP_OK) {
                /* Fall back to ?token=... query param. */
                char query[160];
                if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
                    httpd_query_key_value(query, "token", got, sizeof(got));
                }
            }

            if (std::strcmp(got, expected) != 0) {
                ESP_LOGW(kTag, "Rejected request to %s: bad/missing admin token", req->uri);
                return false;
            }
            return true;
        }
        esp_err_t OtaGetBoundary(httpd_req_t *req, char *out, size_t out_size)
        {
            char ctype[160];
            if (httpd_req_get_hdr_value_str(req, "Content-Type", ctype, sizeof(ctype)) != ESP_OK) {
                return ESP_FAIL;
            }
            const char *key = "boundary=";
            char *p = strstr(ctype, key);
            if (!p) {
                return ESP_FAIL;
            }
            p += std::strlen(key);
            if (*p == '"') {
                p++;
                char *end = strchr(p, '"');
                if (end) *end = '\0';
            }
            std::snprintf(out, out_size, "--%s", p);
            return ESP_OK;
        }

        int FindHeaderEnd(const char *buf, int len)
        {
            for (int i = 0; i + 3 < len; i++) {
                if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n') {
                    return i + 4;
                }
            }
            return -1;
        }

        /* ============================================================
         * Endpoint handlers
         * ============================================================ */

        esp_err_t HandlerHomeGet(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            if (!wp) return ESP_ERR_INVALID_ARG;

            std::string page = wp->BuildHomePage();
            httpd_resp_set_type(req, "text/html");

            constexpr size_t kChunkSize = 512;
            const char *p   = page.c_str();
            size_t      rem = page.size();
            esp_err_t   ret = ESP_OK;

            while (rem > 0) {
                size_t n = rem < kChunkSize ? rem : kChunkSize;
                if (httpd_resp_send_chunk(req, p, static_cast<ssize_t>(n)) != ESP_OK) {
                    ret = ESP_FAIL;
                    break;
                }
                p   += n;
                rem -= n;
            }
            httpd_resp_send_chunk(req, nullptr, 0);
            return ret;
        }

        /* POST / - receive ssid+password from /scan form, notify waiting task */
        esp_err_t HandlerWifiPost(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            if (!wp) return ESP_ERR_INVALID_ARG;

            char body[kPortalBodySize];
            if (RecvBody(req, body, sizeof(body)) != ESP_OK) return ESP_FAIL;

            bool got_ssid = ParsePost(*wp, body);
            ESP_LOGI(kTag, "WiFi POST: ssid='%s'", reinterpret_cast<const char *>(wp->StaConfig().ssid));

            httpd_resp_set_hdr(req, "Connection", "close");
            httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);

            /* Only wake up ConfigViaAp() if an actual SSID was submitted. */
            if (got_ssid) {
                wp->NotifyWifiSubmitted();
            } else {
                ESP_LOGW(kTag, "WiFi POST with empty SSID, ignoring");
            }
            return ESP_OK;
        }

        esp_err_t HandlerScanGet(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            if (!wp) return ESP_ERR_INVALID_ARG;

            std::string inject = BuildScanInjectScript(wp->ScanMaxCount());

            httpd_resp_set_type(req, "text/html");
            httpd_resp_send_chunk(req, WP_PAGE_PART1, HTTPD_RESP_USE_STRLEN);
            httpd_resp_send_chunk(req, inject.c_str(), HTTPD_RESP_USE_STRLEN);
            httpd_resp_send_chunk(req, WP_PAGE_PART2, HTTPD_RESP_USE_STRLEN);
            httpd_resp_send_chunk(req, nullptr, 0);
            return ESP_OK;
        }

        esp_err_t HandlerConfigGet(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            if (!wp) return ESP_ERR_INVALID_ARG;

            if (wp->page().count() == 0) {
                httpd_resp_set_status(req, "302 Temporary Redirect");
                httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
                httpd_resp_send(req, nullptr, 0);
                return ESP_OK;
            }

            std::string page = wp->page().BuildConfigInject();
            httpd_resp_set_type(req, "text/html");
            httpd_resp_send(req, page.c_str(), HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }

        esp_err_t HandlerConfigSavePost(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            if (!wp) return ESP_ERR_INVALID_ARG;

            char body[kPortalBodySize];
            if (RecvBody(req, body, sizeof(body)) != ESP_OK) return ESP_FAIL;

            ParsePost(*wp, body);   // return value intentionally unused here
            ESP_LOGI(kTag, "/configsave: %u params updated", static_cast<unsigned>(wp->page().count()));

            httpd_resp_set_hdr(req, "Connection", "close");
            httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);

            /* Deliberately does NOT call NotifyWifiSubmitted() - saving extra params
            * is not the same event as submitting WiFi credentials. Previously this
            * unblocked ConfigViaAp() prematurely and could report Status::Ok with
            * no actual connection. */
            return ESP_OK;
        }

        esp_err_t HandlerResetGet(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            if (!wp || !CheckAdminToken(req, wp)) {
                httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized");
                return ESP_FAIL;
            }
            httpd_resp_set_hdr(req, "Connection", "close");
            httpd_resp_send(req, "Resetting...", HTTPD_RESP_USE_STRLEN);
            vTaskDelay(pdMS_TO_TICKS(500));
            esp_restart();
            return ESP_OK;
        }

        esp_err_t HandlerOtaGet(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            if (!wp || !CheckAdminToken(req, wp)) {
                httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized");
                return ESP_FAIL;
            }
            httpd_resp_set_type(req, "text/html");
            httpd_resp_send(req, OTA_HTML, HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }


        /* POST /ota - receive firmware via multipart/form-data and flash it. */
        esp_err_t HandlerOtaPost(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            if (!wp) return ESP_ERR_INVALID_ARG;
            if (!CheckAdminToken(req, wp)) {
                httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized");
                return ESP_FAIL;
            }

            if (req->content_len <= 0) {
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
                return ESP_FAIL;
            }

            char boundary[96];
            if (OtaGetBoundary(req, boundary, sizeof(boundary)) != ESP_OK) {
                ESP_LOGE(kTag, "/ota: missing or malformed boundary");
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad Content-Type");
                return ESP_FAIL;
            }
            size_t boundary_len = std::strlen(boundary);
            ESP_LOGI(kTag, "/ota: boundary='%s' content_len=%d", boundary, req->content_len);

            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(nullptr);
            if (!update_partition) {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No OTA partition");
                return ESP_FAIL;
            }
            ESP_LOGI(kTag, "/ota: writing to partition '%s' at 0x%lx",
                     update_partition->label, static_cast<unsigned long>(update_partition->address));

            esp_ota_handle_t ota_handle = 0;
            esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
            if (err != ESP_OK) {
                ESP_LOGE(kTag, "esp_ota_begin failed: %s", esp_err_to_name(err));
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "esp_ota_begin failed");
                return ESP_FAIL;
            }

            char  *buf            = static_cast<char *>(std::malloc(kOtaRecvBufSize));
            bool   header_skipped = false;
            bool   write_failed   = false;
            int    remaining      = req->content_len;
            size_t total_written  = 0;

            char  *tail     = static_cast<char *>(std::malloc(kOtaTailMargin + boundary_len + 8));
            size_t tail_len = 0;

            if (!buf || !tail) {
                esp_ota_abort(ota_handle);
                std::free(buf); std::free(tail);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OOM");
                return ESP_FAIL;
            }

            while (remaining > 0 && !write_failed) {
                int to_read = MIN(remaining, static_cast<int>(kOtaRecvBufSize));
                int recv_len = httpd_req_recv(req, buf, to_read);
                if (recv_len <= 0) {
                    if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) continue;
                    ESP_LOGE(kTag, "/ota: recv error %d", recv_len);
                    write_failed = true;
                    break;
                }
                remaining -= recv_len;

                char *data     = buf;
                int   data_len = recv_len;

                if (!header_skipped) {
                    int header_end = FindHeaderEnd(buf, recv_len);
                    if (header_end < 0) {
                        ESP_LOGE(kTag, "/ota: multipart header not found in first chunk");
                        write_failed = true;
                        break;
                    }
                    data     = buf + header_end;
                    data_len = recv_len - header_end;
                    header_skipped = true;
                }

                size_t hold_back = boundary_len + kOtaTailMargin;

                if (tail_len > 0) {
                    if (esp_ota_write(ota_handle, tail, tail_len) != ESP_OK) {
                        write_failed = true;
                        break;
                    }
                    total_written += tail_len;
                    tail_len = 0;
                }

                if (static_cast<size_t>(data_len) <= hold_back) {
                    std::memcpy(tail, data, data_len);
                    tail_len = data_len;
                } else {
                    size_t flush_len = data_len - hold_back;
                    if (esp_ota_write(ota_handle, data, flush_len) != ESP_OK) {
                        write_failed = true;
                        break;
                    }
                    total_written += flush_len;
                    std::memcpy(tail, data + flush_len, hold_back);
                    tail_len = hold_back;
                }
            }

            if (!write_failed && tail_len > 0) {
                const char *boundary_marker = boundary;
                char *boundary_pos = nullptr;
                for (size_t i = 0; i + boundary_len <= tail_len; i++) {
                    if (std::memcmp(tail + i, boundary_marker, boundary_len) == 0) {
                        boundary_pos = tail + i;
                        break;
                    }
                }
                size_t real_len = boundary_pos ? static_cast<size_t>(boundary_pos - tail) : tail_len;

                if (real_len >= 2 && tail[real_len - 2] == '\r' && tail[real_len - 1] == '\n') {
                    real_len -= 2;
                }

                if (real_len > 0 && esp_ota_write(ota_handle, tail, real_len) != ESP_OK) {
                    write_failed = true;
                } else {
                    total_written += real_len;
                }
            }

            std::free(buf);
            std::free(tail);

            if (write_failed) {
                esp_ota_abort(ota_handle);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash write failed");
                return ESP_FAIL;
            }

            ESP_LOGI(kTag, "/ota: %u bytes written, finalizing", static_cast<unsigned>(total_written));

            err = esp_ota_end(ota_handle);
            if (err != ESP_OK) {
                ESP_LOGE(kTag, "esp_ota_end failed: %s", esp_err_to_name(err));
                const char *msg = (err == ESP_ERR_OTA_VALIDATE_FAILED) ? "Image validation failed" : "esp_ota_end failed";
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, msg);
                return ESP_FAIL;
            }

            err = esp_ota_set_boot_partition(update_partition);
            if (err != ESP_OK) {
                ESP_LOGE(kTag, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "set_boot_partition failed");
                return ESP_FAIL;
            }

            httpd_resp_set_hdr(req, "Connection", "close");
            httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);

            ESP_LOGI(kTag, "/ota: success, rebooting in 1s");
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
            return ESP_OK;
        }

        esp_err_t HandlerCaptiveProbe(httpd_req_t *req)
        {
            auto *wp = static_cast<Manager *>(req->user_ctx);
            std::string location = "http://" + (wp ? wp->CurrentIpString() : std::string("192.168.4.1")) + "/";

            httpd_resp_set_status(req, "302 Temporary Redirect");
            httpd_resp_set_hdr(req, "Location", location.c_str());
            httpd_resp_send(req, nullptr, 0);
            return ESP_OK;
        }

        esp_err_t Handler404(httpd_req_t *req, httpd_err_code_t /*err*/)
        {
            httpd_resp_set_status(req, "302 Temporary Redirect");
            httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
            httpd_resp_send(req, nullptr, 0);
            return ESP_OK;
        }

        /* ============================================================
         * Endpoint table
         * ============================================================ */

#define WP_ENDPOINT(m, u, h) httpd_uri_t{ .uri = (u), .method = (m), .handler = (h), .user_ctx = nullptr }

        httpd_uri_t g_endpoints[] = {
            WP_ENDPOINT(HTTP_GET,  "/",                    HandlerHomeGet),
            WP_ENDPOINT(HTTP_POST, "/",                    HandlerWifiPost),
            WP_ENDPOINT(HTTP_GET,  "/scan",                HandlerScanGet),
            WP_ENDPOINT(HTTP_GET,  "/config",              HandlerConfigGet),
            WP_ENDPOINT(HTTP_POST, "/configsave",          HandlerConfigSavePost),
            WP_ENDPOINT(HTTP_GET,  "/reset",               HandlerResetGet),
            WP_ENDPOINT(HTTP_POST, "/ota",                 HandlerOtaPost),
            WP_ENDPOINT(HTTP_GET,  "/ota",                 HandlerOtaGet),
            WP_ENDPOINT(HTTP_GET,  "/hotspot-detect.html", HandlerCaptiveProbe), /* Apple   */
            WP_ENDPOINT(HTTP_GET,  "/generate_204",        HandlerCaptiveProbe), /* Android */
            WP_ENDPOINT(HTTP_GET,  "/connecttest.txt",     HandlerCaptiveProbe), /* Windows */
            WP_ENDPOINT(HTTP_GET,  "/ncsi.txt",            HandlerCaptiveProbe), /* Windows */
        };
        constexpr size_t kEndpointCount = sizeof(g_endpoints) / sizeof(g_endpoints[0]);

#undef WP_ENDPOINT

    } // namespace

    /* ============================================================
     * Manager members
     * ============================================================ */

    std::string Manager::CurrentIpString() const
    {
        if (priv_ && priv_->netif) {
            esp_netif_ip_info_t ip_info{};
            if (esp_netif_get_ip_info(priv_->netif, &ip_info) == ESP_OK) {
                char buf[INET_ADDRSTRLEN];
                inet_ntoa_r(ip_info.ip.addr, buf, sizeof(buf));
                return buf;
            }
        }
        return "192.168.4.1";
    }

    std::string Manager::BuildHomePage()
    {
        bool connected = IsConnected();
        std::string ip_str  = CurrentIpString();
        std::string ssid_str = "--";
        char host_str[64] = "sharp-edge.local";
        const char *mode_str = "AP";

        if (priv_ && priv_->netif) {
            const char *h = nullptr;
            if (esp_netif_get_hostname(priv_->netif, &h) == ESP_OK && h) {
                std::snprintf(host_str, sizeof(host_str), "%s", h);
            }
        }

        if (connected) {
            wifi_ap_record_t ap{};
            if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
                ssid_str = reinterpret_cast<const char *>(ap.ssid);
            }
            mode_str = "STA";
        }

        std::string page(HOME_HTML);

        auto replace_all = [&page](const char *needle, const std::string &val) {
            size_t pos = page.find(needle);
            if (pos != std::string::npos) {
                page.replace(pos, std::strlen(needle), val);
            }
        };

        replace_all("%HOSTNAME%",  host_str);
        replace_all("%IP%",        ip_str);
        replace_all("%SSID%",      ssid_str);
        replace_all("%MODE%",      mode_str);
        replace_all("%CONNECTED%", connected ? "true" : "false");
        replace_all("%SCANCOUNT%", "0"); /* scan count isn't tracked on the home page */

        return page;
    }

    Status Manager::StartWebServer()
    {
        httpd_config_t cfg   = HTTPD_DEFAULT_CONFIG();
        cfg.stack_size       = 8192;
        cfg.max_open_sockets = 7;
        cfg.lru_purge_enable = true;
        cfg.recv_wait_timeout = 10;
        cfg.send_wait_timeout = 10;
        cfg.max_uri_handlers  = kEndpointCount + 2;

        httpd_handle_t server = nullptr;
        if (httpd_start(&server, &cfg) != ESP_OK) {
            ESP_LOGE(kTag, "httpd_start failed");
            return Status::WifiError;
        }

        for (size_t i = 0; i < kEndpointCount; i++) {
            g_endpoints[i].user_ctx = this;
            httpd_register_uri_handler(server, &g_endpoints[i]);
        }

        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND,          Handler404);
        httpd_register_err_handler(server, HTTPD_405_METHOD_NOT_ALLOWED, Handler404);

        priv_->server = server;
        ESP_LOGI(kTag, "Server started, %u endpoints registered", static_cast<unsigned>(kEndpointCount));
        return Status::Ok;
    }

    void Manager::StopWebServer()
    {
        if (priv_->server) {
            httpd_stop(priv_->server);
            priv_->server = nullptr;
            ESP_LOGI(kTag, "Server stopped");
        }
    }

    Status Manager::SetCaptivePortalUri()
    {
        esp_netif_t *netif = priv_->netif;
        if (!netif) {
            return Status::NetifError;
        }

        std::string uri = "http://" + CurrentIpString();

        esp_err_t ret = esp_netif_dhcps_stop(netif);
        if (ret != ESP_OK && ret != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED) {
            return Status::NetifError;
        }

        ret = esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_CAPTIVEPORTAL_URI,
                                      const_cast<char *>(uri.c_str()), uri.size());
        if (ret != ESP_OK) {
            return Status::NetifError;
        }

        ret = esp_netif_dhcps_start(netif);
        if (ret != ESP_OK && ret != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED) {
            return Status::NetifError;
        }

        ESP_LOGI(kTagDhcp, "Captive portal URI: %s", uri.c_str());
        return Status::Ok;
    }

    void *Manager::StartDns(esp_ip4_addr_t ip)
    {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntoa_r(ip.addr, ip_str, sizeof(ip_str));

        auto *h = new (std::nothrow) DnsHandle{};
        if (!h) {
            return nullptr;
        }

        if (DnsInit(h->dns, ip_str, kDnsPort) != DnsStatus::Ok ||
            DnsStart(h->dns) != DnsStatus::Ok) {
            delete h;
            return nullptr;
        }

        if (xTaskCreate(DnsTaskEntry, "dns_srv", 4096, &h->dns, 5, &h->task) != pdPASS) {
            DnsStop(h->dns);
            delete h;
            return nullptr;
        }

        ESP_LOGI(kTagDns, "Started -> %s:%u", ip_str, kDnsPort);
        return h;
    }

    void Manager::StopDns(void *dns_handle)
    {
        if (!dns_handle) {
            return;
        }
        auto *h = static_cast<DnsHandle *>(dns_handle);
        if (h->task) {
            vTaskDelete(h->task);
            h->task = nullptr;
        }
        DnsStop(h->dns);
        delete h;
        ESP_LOGI(kTagDns, "Stopped");
    }

} /* WiFiPan */