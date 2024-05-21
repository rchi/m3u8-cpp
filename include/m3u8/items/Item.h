#pragma once

#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

#include "AttributeList.h"

#define LOGD std::cerr

using json = nlohmann::json;

typedef enum {
    ItemTypePlaylist,
    ItemTypeStream,
    ItemTypeIframeStream,
    ItemTypeMedia,
    ItemTypeMax,
} ItemType_t;

class Item : public json {
   public:
    class PropertyList : public json {
       public:
        PropertyList(json props = json::object()) : json(json::object())
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
                this->set(toLowerCase(key), value);
            }
        }
        json get(std::string key) { return (*this)[key]; }

        void set(std::string key, json value)
        {
            (*this)[toLowerCase(key)] = value;
        }
    };

   public:
    AttributeList attributeList;
    PropertyList propertyList;

   public:
    Item(json data = json::object())
        : json(json::object()),
          attributeList(data),
          propertyList(json::object())
    {
        setData(data);
    }
    Item(const char* data)
        : json(json::object()),
          attributeList(json::object()),
          propertyList(json::object())
    {
        setData(json::parse(data));
    }
    virtual ItemType_t itemType() { return ItemTypeMax; };

    void setData(json data)
    {
        for (json::iterator it = data.begin(); it != data.end(); ++it) {
            this->set(it.key(), it.value());
        }
    }

    bool propertiesHasKey(std::string key)
    {
        return propertyList.find(key) != propertyList.end();
    }

    bool attributesHasKey(std::string key)
    {
        return attributeList.find(toLowerCase(key)) != attributeList.end();
    }

    void set(std::string key, json value)
    {
        if (this->propertiesHasKey(key)) {
            // std::cout << key << ":" << value << "\n";
            propertyList[key] = value;
        }
        else if (this->attributesHasKey(key)) {
            attributeList[key] = value;
        }
        else {
            std::cerr << "Invalid property : " << key << std::endl;
        }
        // std::cout << (*this).dump() << "\n";
    }

    json get(std::string key)
    {
        if (this->propertiesHasKey(key)) {
            return propertyList[key];
        }
        else if (this->attributesHasKey(key)) {
            return attributeList[key];
        }
        else {
            return json::object();
        }
    }

    json getBoolean(std::string key)
    {
        json value = this->get(key);
        if (value.is_boolean()) {
            return value;
        }
        else {
            return json::object();
        }
    }

    void setProperty(std::string key, json value)
    {
        if (this->propertiesHasKey(key)) {
            // std::cout << key << ":" << value << "\n";
            propertyList[key] = value;
        }
        else {
            std::cerr << "Invalid property : " << key << std::endl;
        }
        // std::cout << (*this).dump() << "\n";
    }

    json getProperty(std::string key)
    {
        if (this->propertiesHasKey(key)) {
            return propertyList[key];
        }
        else {
            std::cerr << "Invalid property : " << key << std::endl;
            return json::object();
        }
    }

    void setAttribute(std::string key, json value)
    {
        std::string k = toLowerCase(key);
        if (this->attributesHasKey(k)) {
            attributeList[k] = value;
        }
        else {
            std::cerr << "Invalid property : " << key << std::endl;
        }
        // std::cout << (*this).dump() << "\n";
    }

    json attribute(std::string key)
    {
        std::string k = toLowerCase(key);
        if (this->attributesHasKey(k)) {
            return attributeList[k];
        }
        else {
            std::cerr << "Invalid property : " << key << std::endl;
            return json::object();
        }
    }

    std::string toString()
    {
        return std::string("") + dump() + "\n" + "{" + attributeList.dump(4)
               + propertyList.dump(4) + "}";
    }

   private:
    static std::string toLowerCase(const std::string& str)
    {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return lowerStr;
    }
};
