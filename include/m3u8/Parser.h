#pragma once

#include <m3u8/M3u8.h>

#include <regex>
#include <sstream>

using json = nlohmann::json;

class M3UParser {
public:
    typedef enum {
        ItemCallback,
        M3u8Callback,
    } CallbackType_t;

public:
    using CallbackFunc
        = std::function<void(CallbackType_t type, std::shared_ptr<void>)>;

    M3UParser()
        : callback(nullptr),
          linesRead(0),
          m3u(std::make_shared<M3u8>()),
          playlistDiscontinuity(false)
    {
        tagHandlers = {
            {"EXTINF",
             [this](const std::string &data) { return parseInf(data); }},
            {"EXT-X-DISCONTINUITY",
             [this](const std::string &data) {
                 return parseDiscontinuity(data);
             }},
            {"EXT-X-BYTERANGE",
             [this](const std::string &data) { return parseByteRange(data); }},
            {"EXT-X-STREAM-INF",
             [this](const std::string &data) { return parseStreamInf(data); }},
            {"EXT-X-I-FRAME-STREAM-INF",
             [this](const std::string &data) {
                 return parseIFrameStreamInf(data);
             }},
            {"EXT-X-MEDIA",
             [this](const std::string &data) { return parseMedia(data); }}};
    }

    void setCallback(CallbackFunc callback) { this->callback = callback; }

    void parse(std::string line)
    {
        line = trim(line);
        if (linesRead == 0) {
            if (line != "#EXTM3U") {
                // emit 'error' event
                throw std::runtime_error("Non-valid M3u8 file. First line: "
                                         + line);
            }
            linesRead++;
            return;
        }
        if (line == "#EXT-X-ENDLIST" || line.empty()) {
            if (line == "#EXT-X-ENDLIST") {
                m3u->set("playlistType", "VOD");
            }
            // std::cout << m3u->toString() << std::endl;
            return;
        }
        if (line[0] == '#') {
            parseLine(line);
        }
        else {
            // if (m3u->getCurrentItem()->get("uri") != nullptr) {
            //     m3u->addItem(std::make_shared<PlaylistItem>());
            // }
            m3u->setCurrent("uri", line);
            // emit 'item' event
            if (callback != nullptr) {
                callback(ItemCallback, getCurrentItem());
            }
        }
        linesRead++;
    }

    std::shared_ptr<M3u8> m3u8()
    {
        if (m3u == nullptr) {
            std::cerr << "m3u is null. Check if Parser destroyed." << std::endl;
            return std::make_shared<M3u8>();
        }
        return m3u;
    }

    void merge(std::shared_ptr<M3u8> from, size_t max = 0)
    {
        if (m3u8() == nullptr) {
            std::cerr << "m3u8() is null" << std::endl;
            return;
        }
        else {
            m3u8()->merge(from, max, [this](std::shared_ptr<Item> item) {
                if (callback != nullptr) {
                    callback(ItemCallback, item);
                }
            });
        }
    }

protected:
    std::shared_ptr<Item> getCurrentItem() { return m3u->getCurrentItem(); }

    void parseLine(std::string line)
    {
        std::istringstream iss(line.substr(1));
        std::string tag, data = "";
        if (!std::getline(iss, tag, ':') || !std::getline(iss, data)) {
            tag = iss.str();
        }
        // std::cout << line << "->" << tag << ":" << data << "\n";
        auto handler = tagHandlers.find(tag);
        if (handler != tagHandlers.end()) {
            handler->second(data);
        }
        else {
            m3u->set(tag, data);
        }
    }

    void parseInf(std::string data)
    {
        // std::cout << data << "\n";
        std::shared_ptr<Item> item = std::make_shared<PlaylistItem>();
        m3u->addItem(item);

        std::vector<std::string> dataSplit = split(data, ',');
        m3u->setCurrent("duration", std::stof(dataSplit[0]));
        if (dataSplit.size() > 1) {
            m3u->setCurrent("title", dataSplit[1]);
        }
        if (playlistDiscontinuity) {
            m3u->setCurrent("discontinuity", true);
            playlistDiscontinuity = false;
        }
        // std::cout << "parseInf: "
        //           << data << "\n" << getCurrentItem()->toString() << "\n";
    }

    void parseDiscontinuity(std::string data)
    {
        // std::cout << data << "\n";
        playlistDiscontinuity = true;
    }

    void parseByteRange(std::string data)
    {
        // std::cout << data << "\n";
        m3u->setCurrent("byteRange", data);
    }

    void parseStreamInf(std::string data)
    {
        // std::cout << data << "\n";
        std::shared_ptr<Item> item
            = std::make_shared<StreamItem>(parseAttributes(data));
        m3u->addItem(item);
    }

    void parseIFrameStreamInf(std::string data)
    {
        std::shared_ptr<Item> item
            = std::make_shared<IframeStreamItem>(parseAttributes(data));
        m3u->addItem(item);
        // emit 'item' event
        if (callback != nullptr) {
            callback(ItemCallback, getCurrentItem());
        }
    }

    void parseMedia(std::string data)
    {
        std::shared_ptr<Item> item
            = std::make_shared<MediaItem>(parseAttributes(data));
        // std::cout << mediaItem->dump(4) << "\n";
        m3u->addItem(item);
        // emit 'item' event
        if (callback != nullptr) {
            callback(ItemCallback, getCurrentItem());
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
    std::shared_ptr<M3u8> m3u;
    bool playlistDiscontinuity;
    std::map<std::string, std::function<void(std::string)>> tagHandlers;
};
