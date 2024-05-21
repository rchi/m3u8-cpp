#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <m3u8/items/AttributeList.h>
#include <m3u8/items/IframeStreamItem.h>
#include <m3u8/items/Item.h>
#include <m3u8/items/MediaItem.h>
#include <m3u8/items/PlaylistItem.h>
#include <m3u8/items/StreamItem.h>

class PlaylistItem;
class StreamItem;
class IframeStreamItem;
class MediaItem;

using json = nlohmann::json;

class M3u8 : public json {
   public:
    M3u8() : current(nullptr)
    {
        items[ItemTypePlaylist]     = std::vector<std::shared_ptr<Item>>();
        items[ItemTypeStream]       = std::vector<std::shared_ptr<Item>>();
        items[ItemTypeIframeStream] = std::vector<std::shared_ptr<Item>>();
        items[ItemTypeMedia]        = std::vector<std::shared_ptr<Item>>();
        (*this)["properties"]       = json::object();
        (*this)["properties"]["iframesOnly"]         = json::object();
        (*this)["properties"]["independentSegments"] = json::object();
        (*this)["properties"]["targetDuration"]      = json::object();
        (*this)["properties"]["mediaSequence"]       = json::object();
        (*this)["properties"]["version"]             = json::object();
    }

    json get(std::string key)
    {
        json object;
        if (key == "EXT-X-ALLOW-CACHE") {
            object = get("allowCache");
        }
        else if (key == "EXT-X-I-FRAMES-ONLY") {
            object = get("iframesOnly");
        }
        else if (key == "EXT-X-INDEPENDENT-SEGMENTS") {
            object = get("independentSegments");
        }
        else if (key == "EXT-X-MEDIA-SEQUENCE") {
            object = get("mediaSequence");
        }
        else if (key == "EXT-X-PLAYLIST-TYPE") {
            object = get("playlistType");
        }
        else if (key == "EXT-X-TARGETDURATION") {
            object = get("targetDuration");
        }
        else if (key == "EXT-X-VERSION") {
            object = get("version");
        }
        else {
            object = (*this)["properties"][key];
        }
        return object;
    }

    void set(std::string key, json value)
    {
        if (key == "EXT-X-ALLOW-CACHE") {
            set("allowCache", value);
        }
        else if (key == "EXT-X-I-FRAMES-ONLY") {
            set("iframesOnly", true);
        }
        else if (key == "EXT-X-INDEPENDENT-SEGMENTS") {
            set("independentSegments", value);
        }
        else if (key == "EXT-X-MEDIA-SEQUENCE") {
            set("mediaSequence", std::stoi(value.get<std::string>()));
        }
        else if (key == "EXT-X-PLAYLIST-TYPE") {
            set("playlistType", value);
        }
        else if (key == "EXT-X-TARGETDURATION") {
            set("targetDuration", std::stoi(value.get<std::string>()));
        }
        else if (key == "EXT-X-VERSION") {
            set("version", std::stoi(value.get<std::string>()));
        }
        else {
            (*this)["properties"][key] = value;
        }
    }

    std::shared_ptr<Item> getCurrentItem() { return current; }

    void setCurrent(std::string key, json value)
    {
        // Implementation depends on your application
        if (getCurrentItem() != nullptr) {
            getCurrentItem()->propertyList[key] = value;
        }
    }

    std::shared_ptr<Item> addItem(std::shared_ptr<Item> item)
    {
        items[item->itemType()].push_back(item);
        current = item;
        // std::cout << item->dump(4) << "\n";
        return current;
    }

    void addPlaylistItem(std::shared_ptr<PlaylistItem> item)
    {
        items[ItemTypePlaylist].push_back(item);
    }

    void removePlaylistItem(int index)
    {
        if (index >= items[ItemTypePlaylist].size()) {
            throw std::out_of_range("Index out of range");
        }
        items[ItemTypePlaylist].erase(items[ItemTypePlaylist].begin() + index);
    }

    std::shared_ptr<Item> getItem(ItemType_t type, int index)
    {
        return std::static_pointer_cast<Item>(items[type][index]);
    }

    void addMediaItem(std::shared_ptr<MediaItem> item)
    {
        items[ItemTypeMedia].push_back(std::shared_ptr<Item>(item));
    }

    void addStreamItem(std::shared_ptr<StreamItem> item)
    {
        items[ItemTypeStream].push_back(std::shared_ptr<Item>(item));
    }

    void addIframeStreamItem(std::shared_ptr<IframeStreamItem> item)
    {
        items[ItemTypeIframeStream].push_back(std::shared_ptr<Item>(item));
    }

    std::vector<double> domainDurations()
    {
        std::vector<double> durations = {0};
        int index                     = 0;
        for (const auto& item : items[ItemTypePlaylist]) {
            if (item->propertyList["discontinuity"].is_boolean()) {
                if ((bool)item->propertyList["discontinuity"]) {
                    durations.push_back(0);
                    index++;
                }
            }
            if (item->propertyList["duration"].is_number()) {
                double duration = item->propertyList["duration"];
                durations[index] += duration;
            }
        }
        return durations;
    }

    double totalDuration()
    {
        double total = 0;
        for (const auto& item : items[ItemTypePlaylist]) {
            if (item->propertyList["duration"].is_number()) {
                double duration = item->propertyList["duration"];
                total += duration;
            }
        }
        return total;
    }

    void merge(M3u8* m3u)
    {
        double m3uTgt  = 0;
        double thisTgt = 0;
        if (m3u->get("targetDuration").is_number()) {
            m3uTgt = m3u->get("targetDuration");
        }
        if (this->get("targetDuration").is_number()) {
            thisTgt = this->get("targetDuration");
        }
        if (m3uTgt > thisTgt) {
            this->set("targetDuration", m3u->get("targetDuration"));
        }
        m3u->items[ItemTypePlaylist][0]->propertyList["discontinuity"] = true;
        items[ItemTypePlaylist].insert(items[ItemTypePlaylist].end(),
                                       m3u->items[ItemTypePlaylist].begin(),
                                       m3u->items[ItemTypePlaylist].end());
    }

    std::string toString()
    {
        std::string str = this->dump(4);
        for (auto i : items) {
            for (auto j : i.second) {
                str += j->toString() + "\n";
                // std::cout << j.get() << "\n";
            }
        }

        return str;
    }

   public:
    std::map<ItemType_t, std::vector<std::shared_ptr<Item>>> items;

   private:
    std::shared_ptr<Item> current;
};
