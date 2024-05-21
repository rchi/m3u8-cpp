#pragma once

#include <string>

#include <m3u8/items/StreamItem.h>

class IframeStreamItem : public StreamItem {
   public:
    IframeStreamItem(json data = json::object()) : StreamItem(data)
    {
        propertyList.erase("uri");
    }
    IframeStreamItem(const char* data) : StreamItem(data) {}
    ItemType_t itemType() { return ItemTypeIframeStream; };
};
