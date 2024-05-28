#pragma once

#include <m3u8/items/AttributeList.h>
#include <m3u8/items/IframeStreamItem.h>
#include <m3u8/items/Item.h>
#include <m3u8/items/MediaItem.h>
#include <m3u8/items/PlaylistItem.h>
#include <m3u8/items/StreamItem.h>

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class PlaylistItem;
class StreamItem;
class IframeStreamItem;
class MediaItem;

using json = nlohmann::json;

class M3u8 : public JsonBase {
    friend class M3UParser;

public:
    M3u8() : current(nullptr)
    {
        items[ItemTypePlaylist]     = std::vector<std::shared_ptr<Item>>();
        items[ItemTypeStream]       = std::vector<std::shared_ptr<Item>>();
        items[ItemTypeIframeStream] = std::vector<std::shared_ptr<Item>>();
        items[ItemTypeMedia]        = std::vector<std::shared_ptr<Item>>();
        (*this)["properties"]       = json::object();
        (*this)["properties"]["iframesOnly"]         = json::boolean_t();
        (*this)["properties"]["independentSegments"] = json::object();
        (*this)["properties"]["targetDuration"]      = json::number_integer_t();
        (*this)["properties"]["mediaSequence"]       = json::number_integer_t();
        (*this)["properties"]["version"]             = json::number_integer_t();
    }

    std::map<std::string, std::string> keyMap
        = {{"EXT-X-ALLOW-CACHE", "allowCache"},
           {"EXT-X-I-FRAMES-ONLY", "iframesOnly"},
           {"EXT-X-INDEPENDENT-SEGMENTS", "independentSegments"},
           {"EXT-X-MEDIA-SEQUENCE", "mediaSequence"},
           {"EXT-X-PLAYLIST-TYPE", "playlistType"},
           {"EXT-X-TARGETDURATION", "targetDuration"},
           {"EXT-X-VERSION", "version"}};

    json get(std::string key)
    {
        if (keyMap.count(key) > 0) {
            return get(keyMap[key]);
        }
        else {
            return (*this)["properties"][key];
        }
    }

    void set(std::string key, json value)
    {
        if (keyMap.count(key) > 0) {
            set(keyMap[key], value);
        }
        else {
            JsonBase::set(&((*this)["properties"]), key, value);
        }
    }

    size_t getItemCount(ItemType_t type) { return items[type].size(); }

    std::shared_ptr<Item> getItem(ItemType_t type, int index)
    {
        return std::static_pointer_cast<Item>(items[type][index]);
    }

    std::shared_ptr<Item> getLastItem(ItemType_t type)
    {
        if (items[type].size() == 0) {
            return nullptr;
        }
        return items[type].back();
    }

    std::vector<double> domainDurations()
    {
        std::vector<double> durations = {0};
        int index                     = 0;
        for (const auto &item : items[ItemTypePlaylist]) {
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
        for (const auto &item : items[ItemTypePlaylist]) {
            if (item->propertyList["duration"].is_number()) {
                double duration = item->propertyList["duration"];
                total += duration;
            }
        }
        return total;
    }

    void mergeProperties(M3u8 *from)
    {
        for (json::iterator it = (*from)["properties"].begin();
             it != (*from)["properties"].end(); ++it) {
            double m3uTgt  = 0;
            double thisTgt = 0;
            if (it.key() == "targetDuration") {
                if (from->get("targetDuration").is_number()) {
                    m3uTgt = from->get("targetDuration");
                }
                if (get("targetDuration").is_number()) {
                    thisTgt = get("targetDuration");
                }
                if (m3uTgt > thisTgt) {
                    set("targetDuration", from->get("targetDuration"));
                }
            }
            else {
                set(it.key(), it.value());
            }
        }
    }

    void merge(std::shared_ptr<M3u8> from, size_t max,
               std::function<void(std::shared_ptr<Item>)> callback = nullptr)
    {
        mergeProperties(from.get());

        for (auto i : items) {
            for (const auto &newItem : from->items[i.first]) {
                auto it
                    = std::find_if(items[i.first].begin(), items[i.first].end(),
                                   [&newItem](const auto &existingItem) {
                                       return existingItem->propertyList["uri"]
                                              == newItem->propertyList["uri"];
                                   });
                if (it != items[i.first].end()) {
                    *it = newItem;
                }
                else {
                    items[i.first].push_back(newItem);
                    if (callback != nullptr) {
                        callback(newItem);
                    }
                }
            }

            if (max > 0 && items[i.first].size() > max) {
                items[i.first].erase(
                    items[i.first].begin(),
                    items[i.first].begin() + (items[i.first].size() - max));
            }
        }
        // std::cout << toString() << std::endl;
    }

    std::string toString()
    {
        std::string str = dump(4);
        for (auto i : items) {
            str += "\n{ " + Item::StringType(i.first)
                   + "Count: " + std::to_string(i.second.size()) + ", ";
            for (auto j : i.second) {
                str += j->toString() + "\n";
                // std::cout << j.get() << "\n";
            }
            str += "}\n";
        }

        return str;
    }

public:
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
        if (index >= getItemCount(ItemTypePlaylist)) {
            throw std::out_of_range("Index out of range");
        }
        items[ItemTypePlaylist].erase(items[ItemTypePlaylist].begin() + index);
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

public:
    std::map<ItemType_t, std::vector<std::shared_ptr<Item>>> items;

private:
    std::shared_ptr<Item> current;
};
