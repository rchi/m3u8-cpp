#pragma once
#include <m3u8/items/Item.h>

class MediaItem : public Item {
public:
    MediaItem(json data = json::object()) : Item(data)
    {
        propertyList.erase("uri");
    }
    MediaItem(const char* data) : Item(data) {}
    ItemType_t itemType() { return ItemTypeMedia; };
};