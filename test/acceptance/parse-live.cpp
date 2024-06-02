#include <gtest/gtest.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "m3u8/UrlParser.h"

using json = nlohmann::json;

class ParserTestLive : public ::testing::Test {
protected:
    std::string generateContent(int f)
    {
        int index           = f;
        std::string context = "";
        context += "#EXTM3U\n";
        context += "#EXT-X-VERSION:3\n";
        context += "#EXT-X-ALLOW-CACHE:NO\n";
        context += "#EXT-X-TARGETDURATION:5\n";
        context += "#EXT-X-MEDIA-SEQUENCE:" + std::to_string(index) + "\n";
        for (int i = f; i < f + 3; i++) {
            context += "#EXTINF:5.000,\n";
            context
                += std::to_string(i) + "_" + std::to_string(index++) + ".ts\n";
        }
        return context;
    }
};

TEST_F(ParserTestLive, continue)
{
    bool end_received                 = false;
    int index                         = 1;
    std::shared_ptr<UrlParser> parser = std::make_shared<UrlParser>();
    parser->setCallback([&](UrlParser::CallbackType_t type,
                            std::shared_ptr<void> arg) {
        if (type == UrlParser::ItemCallback) {
            std::shared_ptr<Item> i = std::static_pointer_cast<Item>(arg);
            std::string uri
                = std::to_string(index) + "_" + std::to_string(index) + ".ts";
            EXPECT_EQ(i->get("uri"), uri);
            index++;
        }
        else if (type == UrlParser::M3u8Callback) {
            std::shared_ptr<M3u8> m3u = std::static_pointer_cast<M3u8>(arg);
            // std::cout << m3u->toString() << std::endl;
            if (m3u->get("playlistType") == "VOD") {
                end_received = true;
            }
        }
    });

    // std::cout << parser->m3u8()->toString() << "\n";
    int i;
    for (i = 0; i < 5; i++) {
        std::string content = generateContent(i + 1);
        // std::cout << content << "\n";
        parser->readNext(content, 4);
        // std::cout << parser->m3u8()->toString() << "\n\n\n";
        EXPECT_EQ(parser->m3u8()->getItemCount(ItemTypePlaylist),
                  (i == 0 ? 3 : 4));
        EXPECT_NE(parser->m3u8()->getLastItem(ItemTypePlaylist), nullptr);
        if (parser->m3u8()->getLastItem(ItemTypePlaylist) != nullptr) {
            std::string uri
                = std::to_string(i + 3) + "_" + std::to_string(i + 3) + ".ts";
            EXPECT_EQ(parser->m3u8()->getLastItem(ItemTypePlaylist)->get("uri"),
                      uri);
        }
    }

    std::string content = generateContent(i + 1);
    content += "#EXT-X-ENDLIST\n";
    parser->readNext(content, 4);
    EXPECT_TRUE(end_received);
}
