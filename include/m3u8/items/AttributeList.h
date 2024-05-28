#pragma once

#include <m3u8/items/JsonBase.h>

using json = nlohmann::json;

class AttributeList : public JsonBase {
public:
    AttributeList(json attrs = json::object())
    {
        (*this)["audio"]      = json::object();            // 'quoted-string',
        (*this)["autoselect"] = json::boolean_t();         // 'boolean',
        (*this)["bandwidth"]  = json::number_integer_t();  // 'decimal-integer',
        (*this)["average-bandwidth"]
            = json::number_integer_t();             // 'decimal-integer',
        (*this)["frame-rate"] = json::object();     // 'decimal-floating',
        (*this)["byterange"]  = json::object();     // 'enumerated-string',
        (*this)["channels"]   = json::object();     // 'quoted-string',
        (*this)["codecs"]     = json::object();     // 'quoted-string',
        (*this)["default"]    = json::boolean_t();  // 'boolean',
        (*this)["duration"]   = json::object();     // 'decimal-floating',
        (*this)["forced"]     = json::boolean_t();  // 'boolean',
        (*this)["group-id"]   = json::object();     // 'quoted-string',
        (*this)["language"]   = json::object();     // 'quoted-string',
        (*this)["name"]       = json::object();     // 'quoted-string',
        (*this)["program-id"] = json::number_integer_t();  // 'decimal-integer',
        (*this)["resolution"] = json::array();   // 'decimal-resolution',
        (*this)["subtitles"]  = json::object();  // 'quoted-string',
        (*this)["title"]      = json::object();  // 'enumerated-string',
        (*this)["type"]       = json::object();  // 'enumerated-string',
        (*this)["uri"]        = json::object();  // 'quoted-string',
        (*this)["video"]      = json::object();  // 'quoted-string'

        for (json::iterator it = attrs.begin(); it != attrs.end(); ++it) {
            set(toLowerCase(it.key()), it.value());
        }
    }

    void set(std::string key, json value)
    {
        JsonBase::set(this, toLowerCase(key), value);
    }
};
