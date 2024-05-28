#pragma once

#include <m3u8/items/JsonBase.h>

using json = nlohmann::json;

class PropertyList : public JsonBase {
public:
    PropertyList(json props = json::object()) : JsonBase(json::object())
    {
        (*this)["byteRange"]               = "";
        (*this)["daiPlacementOpportunity"] = json::object();
        (*this)["date"]                    = json::object();
        (*this)["discontinuity"]           = false;
        (*this)["duration"]                = 0.0;
        (*this)["title"]                   = "";
        (*this)["uri"]                     = "";

        for (json::iterator it = props.begin(); it != props.end(); ++it) {
            std::string key   = it.key();
            std::string value = it.value();
            set(toLowerCase(key), value);
        }
    }

    void set(std::string key, json value) { (*this)[toLowerCase(key)] = value; }
};
