#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <m3u8/items/Item.h>

using json = nlohmann::json;

class PlaylistItem : public Item {
   public:
    PlaylistItem(json data = json::object()) : Item(data) {}
    PlaylistItem(const char* data) : Item(data) {}
    ItemType_t itemType() { return ItemTypePlaylist; };
};