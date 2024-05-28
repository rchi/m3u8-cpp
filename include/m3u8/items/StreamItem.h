#pragma once

#include <m3u8/items/Item.h>

class StreamItem : public Item {
public:
    StreamItem(json data = json::object()) : Item(data) {}
    StreamItem(const char* data) : Item(data) {}
    ItemType_t itemType() { return ItemTypeStream; };
};