#pragma once
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class AttributeList : public json {
   public:
    AttributeList(json attrs = json::object())
    {
        (*this)["audio"]             = json::object();  // 'quoted-string',
        (*this)["autoselect"]        = json::boolean_t();  // 'boolean',
        (*this)["bandwidth"]         = json::number_integer_t();  // 'decimal-integer',
        (*this)["average-bandwidth"] = json::number_integer_t();  // 'decimal-integer',
        (*this)["frame-rate"]        = json::object();  // 'decimal-floating',
        (*this)["byterange"]         = json::object();  // 'enumerated-string',
        (*this)["channels"]          = json::object();  // 'quoted-string',
        (*this)["codecs"]            = json::object();  // 'quoted-string',
        (*this)["default"]           = json::boolean_t();  // 'boolean',
        (*this)["duration"]          = json::object();  // 'decimal-floating',
        (*this)["forced"]            = json::boolean_t();  // 'boolean',
        (*this)["group-id"]          = json::object();  // 'quoted-string',
        (*this)["language"]          = json::object();  // 'quoted-string',
        (*this)["name"]              = json::object();  // 'quoted-string',
        (*this)["program-id"]        = json::number_integer_t();  // 'decimal-integer',
        (*this)["resolution"]        = json::array();  // 'decimal-resolution',
        (*this)["subtitles"]         = json::object();  // 'quoted-string',
        (*this)["title"]             = json::object();  // 'enumerated-string',
        (*this)["type"]              = json::object();  // 'enumerated-string',
        (*this)["uri"]               = json::object();  // 'quoted-string',
        (*this)["video"]             = json::object();  // 'quoted-string'

        for (json::iterator it = attrs.begin(); it != attrs.end(); ++it) {
            this->set(toLowerCase(it.key()), it.value());
        }
    }

    json get(std::string key) { return (*this)[toLowerCase(key)]; }

    void set(std::string key, json value) {
        std::string k = toLowerCase(key);
        if((*this)[k].is_number() && value.is_string()) {
            (*this)[k] = std::stoi(value.get<std::string>());
        }
        else if((*this)[k].is_boolean() && value.is_string()) {
            std::string v = toLowerCase(value);
            if(v == "true" || v == "yes") {
                (*this)[k] = true;
            }
            else {
                (*this)[k] = false;
            }
        }
        else if(k == "resolution" && value.is_string()) {
            std::string v = value;
            std::string delimiter = "x";
            size_t pos = 0;
            std::string token;
            int i = 0;
            while ((pos = v.find(delimiter)) != std::string::npos) {
                token = v.substr(0, pos);
                v.erase(0, pos + delimiter.length());
                (*this)[k][i] = std::stoi(token);
                i++;
            }
            (*this)[k][i] = std::stoi(v);
        }
        else {
            (*this)[k] = value;
        }
    }

   private:
    std::string toLowerCase(const std::string& str)
    {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return lowerStr;
    }
};
