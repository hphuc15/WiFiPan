#include "WiFiPan.hpp"
#include "WiFiPan_Html.h"

#include <cstring>
#include <string>

namespace WiFiPan
{
    namespace
    {
        std::string HtmlEscape(const char *s)
        {
            std::string out;
            out.reserve(std::strlen(s));
            for (; *s; ++s)
            {
                switch (*s)
                {
                    case '&':
                        out += "&amp;";
                        break;
                    case '<':
                        out += "&lt;";
                        break;
                    case '>':
                        out += "&gt;";
                        break;
                    case '"':
                        out += "&quot;";
                        break;
                    case '\'':
                        out += "&#39;";
                        break;
                    default:
                        out += *s;
                        break;
                }
            }
            return out;
        }

        /* Appends src to out, escaping '"' and '\' for embedding in a JSON string. */
        void JsonEscapeAppend(std::string &out, const char *s)
        {
            for (; *s; ++s)
            {
                if (*s == '"' || *s == '\\')
                {
                    out += '\\';
                }
                out += *s;
            }
        }

        void CopyField(std::array<char, kFieldLen> &dst, const char *src)
        {
            std::strncpy(dst.data(), src ? src : "", dst.size() - 1);
            dst[dst.size() - 1] = '\0';
        }
    } // namespace

    Status Page::AddParam(const char *id, const char *label, const char *placeholder, const char *value,
                          const char *type, bool required)
    {
        if (used_ >= kMaxParams)
        {
            return Status::NoMem;
        }

        Param &p = params_[used_++];
        p        = Param{};
        CopyField(p.id, id);
        CopyField(p.label, label);
        CopyField(p.placeholder, placeholder);
        CopyField(p.value, value);
        std::strncpy(p.type.data(), type ? type : "text", p.type.size() - 1);
        p.type[p.type.size() - 1] = '\0';
        p.required                = required;
        return Status::Ok;
    }

    const char *Page::GetParam(const char *id) const
    {
        for (size_t i = 0; i < used_; ++i)
        {
            if (std::strcmp(params_[i].id.data(), id) == 0)
            {
                return params_[i].value.data();
            }
        }
        return nullptr;
    }

    bool Page::SetParamValue(const char *id, const char *value)
    {
        for (size_t i = 0; i < used_; ++i)
        {
            if (std::strcmp(params_[i].id.data(), id) == 0)
            {
                CopyField(params_[i].value, value);
                return true;
            }
        }
        return false;
    }

    std::string Page::BuildFieldsHtml() const
    {
        std::string html;

        for (size_t i = 0; i < used_; ++i)
        {
            const Param      &p    = params_[i];
            const std::string eid  = HtmlEscape(p.id.data());
            const std::string elbl = HtmlEscape(p.label.data());
            const std::string eph  = HtmlEscape(p.placeholder.data());
            const std::string eval = HtmlEscape(p.value.data());
            const std::string etyp = HtmlEscape(p.type.data());
            const char       *req  = p.required ? " required" : "";

            html += "<div class=\"form-group\">";
            html += "<label for=\"" + eid + "\">" + elbl + "</label>";

            if (std::strcmp(p.type.data(), "password") == 0)
            {
                html += "<div class=\"password-wrapper\">"
                        "<input type=\"password\" id=\"" +
                        eid + "\" name=\"" + eid + "\" placeholder=\"" + eph + "\" value=\"" + eval + "\"" + req + ">";
                html += "<span class=\"toggle-password\" onclick=\"toggleField('" + eid + "')\">&#128065;</span></div>";
            }
            else
            {
                html += "<input type=\"" + etyp + "\" id=\"" + eid + "\" name=\"" + eid + "\" placeholder=\"" + eph +
                        "\" value=\"" + eval + "\"" + req + ">";
            }

            if (p.placeholder[0] != '\0')
            {
                html += "<div class=\"label-info\">" + eph + "</div>";
            }
            html += "</div>";
        }
        return html;
    }

    std::string Page::BuildConfigInject() const
    {
        std::string out;
        out.reserve(1024);
        out += "<script>window.wifiparam_schema=[";

        for (size_t i = 0; i < used_; ++i)
        {
            const Param &p = params_[i];

            const char *badge_type;
            if (std::strcmp(p.type.data(), "password") == 0)
                badge_type = "pass";
            else if (std::strcmp(p.type.data(), "number") == 0)
                badge_type = "int";
            else if (std::strcmp(p.type.data(), "checkbox") == 0)
                badge_type = "bool";
            else if (std::strcmp(p.type.data(), "select") == 0)
                badge_type = "sel";
            else
                badge_type = "str";

            if (i > 0)
                out += ',';
            out += "{\"key\":\"";
            JsonEscapeAppend(out, p.id.data());
            out += "\",\"type\":\"";
            out += badge_type;
            out += "\",\"label\":\"";
            JsonEscapeAppend(out, p.label.data());
            out += "\",\"default\":\"";
            JsonEscapeAppend(out, p.value.data());
            out += "\",\"desc\":\"";
            JsonEscapeAppend(out, p.placeholder.data()); /* desc reuses placeholder */
            out += "\"}";
        }
        out += "];";

        out += "window.wifiparam_values={";
        for (size_t i = 0; i < used_; ++i)
        {
            const Param &p = params_[i];
            if (i > 0)
                out += ',';
            out += '"';
            JsonEscapeAppend(out, p.id.data());
            out += "\":\"";
            JsonEscapeAppend(out, p.value.data());
            out += '"';
        }
        out += "};</script>";

        out += CONFIG_HTML;
        return out;
    }

} // namespace WiFiPan