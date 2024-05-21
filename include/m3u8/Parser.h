#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <m3u8/M3u8.h>

using json = nlohmann::json;

class M3UParser {
   public:
    typedef enum {
        ItemCallback,
        M3u8Callback,
    } CallbackType_t;

   public:
    M3UParser() : callback(NULL)
    {
        this->linesRead   = 0;
        this->m3u         = new M3u8;
        this->tagHandlers = {
            {"EXTINF",
             [this](const std::string &data) { return this->parseInf(data); }},
            {"EXT-X-DISCONTINUITY",
             [this](const std::string &data) {
                 return this->parseDiscontinuity(data);
             }},
            {"EXT-X-BYTERANGE",
             [this](const std::string &data) {
                 return this->parseByteRange(data);
             }},
            {"EXT-X-STREAM-INF",
             [this](const std::string &data) {
                 return this->parseStreamInf(data);
             }},
            {"EXT-X-I-FRAME-STREAM-INF",
             [this](const std::string &data) {
                 return this->parseIFrameStreamInf(data);
             }},
            {"EXT-X-MEDIA", [this](const std::string &data) {
                 return this->parseMedia(data);
             }}};
    }

    using CallbackFunc = std::function<void(CallbackType_t type, void *)>;
    void setCallback(CallbackFunc callback) { this->callback = callback; }

    void parse(std::string line)
    {
        line = trim(line);
        if (this->linesRead == 0) {
            if (line != "#EXTM3U") {
                // emit 'error' event
                throw std::runtime_error("Non-valid M3u8 file. First line: "
                                         + line);
            }
            this->linesRead++;
            return;
        }
        if (line == "#EXT-X-ENDLIST" || line.empty()) {
            if (line == "#EXT-X-ENDLIST") {
                this->m3u->set("playlistType", "VOD");
            }
            return;
        }
        if (line[0] == '#') {
            this->parseLine(line);
        }
        else {
            // if (this->m3u->getCurrentItem()->get("uri") != nullptr) {
            //     this->m3u->addItem(new PlaylistItem);
            // }
            this->m3u->setCurrent("uri", line);
            // emit 'item' event
            if (callback != NULL) {
                callback(ItemCallback, getCurrentItem().get());
            }
        }
        this->linesRead++;
    }

    void parseLine(std::string line)
    {
        std::istringstream iss(line.substr(1));
        std::string tag, data = "";
        if (!std::getline(iss, tag, ':') || !std::getline(iss, data)) {
            tag = iss.str();
        }
        // std::cout << line << "->" << tag << ":" << data << "\n";
        auto handler = this->tagHandlers.find(tag);
        if (handler != this->tagHandlers.end()) {
            handler->second(data);
        }
        else {
            this->m3u->set(tag, data);
        }
    }

    M3u8 *m3u8() { return m3u; }

    std::shared_ptr<Item> getCurrentItem() { return m3u->getCurrentItem(); }

    void parseInf(std::string data)
    {
        // std::cout << data << "\n";
        std::shared_ptr<Item> item = std::make_shared<PlaylistItem>();
        this->m3u->addItem(item);

        std::vector<std::string> dataSplit = split(data, ',');
        this->m3u->setCurrent("duration", std::stof(dataSplit[0]));
        if (dataSplit.size() > 1) {
            this->m3u->setCurrent("title", dataSplit[1]);
        }
        if (this->playlistDiscontinuity) {
            this->m3u->setCurrent("discontinuity", true);
            this->playlistDiscontinuity = false;
        }
        // std::cout << getCurrentItem()->toString() << "\n";
    }

    void parseDiscontinuity(std::string data)
    {
        // std::cout << data << "\n";
        this->playlistDiscontinuity = true;
    }

    void parseByteRange(std::string data)
    {
        // std::cout << data << "\n";
        this->m3u->setCurrent("byteRange", data);
    }

    void parseStreamInf(std::string data)
    {
        // std::cout << data << "\n";
        std::shared_ptr<Item> item
            = std::make_shared<StreamItem>(this->parseAttributes(data));
        this->m3u->addItem(item);
    }

    void parseIFrameStreamInf(std::string data)
    {
        std::shared_ptr<Item> item
            = std::make_shared<IframeStreamItem>(this->parseAttributes(data));
        this->m3u->addItem(item);
        // emit 'item' event
        if (callback != NULL) {
            callback(ItemCallback, getCurrentItem().get());
        }
    }

    void parseMedia(std::string data)
    {
        std::shared_ptr<Item> item
            = std::make_shared<MediaItem>(this->parseAttributes(data));
        // std::cout << mediaItem->dump(4) << "\n";
        this->m3u->addItem(item);
        // emit 'item' event
        if (callback != NULL) {
            callback(ItemCallback, getCurrentItem().get());
        }
    }

    json parseAttributes(std::string data)
    {
        // std::cout << "parseAttributes: " << data << "\n";

        json result = json::object();
        std::regex r(R"(\s*([^=]+)=("[^"]*"|[^,]*),?)");
        std::smatch match;
        while (std::regex_search(data, match, r)) {
            std::string key   = trim(match[1]);
            std::string value = trim(match[2]);
            if (value.size() >= 2 && value[0] == '"'
                && value[value.size() - 1] == '"') {
                value = value.substr(1, value.size() - 2);
            }
            // std::cout << "parseAttributes: " << key << ":" << value << "\n";
            result[key] = value;
            data        = match.suffix().str();
        }
        // std::cout << result.dump(4) << "\n";
        return result;
    }

   private:
    std::vector<std::string> split(const std::string &s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string trim(const std::string &str)
    {
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

   protected:
    CallbackFunc callback;
   private:
    int linesRead;
    M3u8 *m3u;
    bool playlistDiscontinuity;
    std::map<std::string, std::function<void(std::string)>> tagHandlers;
};
