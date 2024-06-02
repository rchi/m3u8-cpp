#pragma once

#include <m3u8/items/AttributeList.h>
#include <m3u8/items/PropertyList.h>

#include <iostream>
#include <map>

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

public:
    AttributeList attributeList;
    PropertyList propertyList;

public:
    virtual ItemType_t itemType() { return ItemTypeMax; };

    template <typename T = json>
    T get(std::string key)
    {
        if (propertiesHasKey(key)) {
            return propertyList[key].get<T>();
        }
        else if (attributesHasKey(key)) {
            return attributeList[key].get<T>();
        }
        else {
            return T();
        }
    }

    std::string toString()
    {
        return std::string("") + dump() + "\n" + "{" + attributeList.dump(4)
               + propertyList.dump(4) + "}";
    }

    static std::string StringType(ItemType_t type)
    {
        switch (type) {
            case ItemTypePlaylist:
                return "Playlist";
            case ItemTypeStream:
                return "Stream";
            case ItemTypeIframeStream:
                return "IframeStream";
            case ItemTypeMedia:
                return "Media";
            default:
                return "Unknown";
        }
    }

protected:
    void setData(json data)
    {
        for (json::iterator it = data.begin(); it != data.end(); ++it) {
            set(it.key(), it.value());
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
        if (propertiesHasKey(key)) {
            // std::cout << key << ":" << value << "\n";
            propertyList[key] = value;
        }
        else if (attributesHasKey(key)) {
            attributeList[key] = value;
        }
        else {
            std::cerr << "Invalid property : " << key << std::endl;
        }
        // std::cout << (*this).dump() << "\n";
    }

    void setProperty(std::string key, json value)
    {
        if (propertiesHasKey(key)) {
            // std::cout << key << ":" << value << "\n";
            propertyList[key] = value;
        }
        else {
            std::cerr << "Invalid property : " << key << std::endl;
        }
        // std::cout << (*this).dump() << "\n";
    }

    void setAttribute(std::string key, json value)
    {
        std::string k = toLowerCase(key);
        if (attributesHasKey(k)) {
            attributeList[k] = value;
        }
        else {
            std::cerr << "Invalid property : " << key << std::endl;
        }
        // std::cout << (*this).dump() << "\n";
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
