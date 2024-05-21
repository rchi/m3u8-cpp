#include <gtest/gtest.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include "m3u8/FileParser.h"

extern std::string path;

using json = nlohmann::json;

class ParserTestPlaylist : public ::testing::Test {
protected:
    FileParser* getParser() {
        bool done = false;
        int count = 0;
        FileParser* parser = new FileParser;
        parser->setCallback([&](FileParser::CallbackType_t type, void*) {
            if(type == FileParser::ItemCallback) {
                count++;
            }
            else if(type == FileParser::M3u8Callback) {
                EXPECT_EQ(count, 17);
                done = true;
            }
        });

        parser->parseFile(path + "/playlist.m3u8");
        // std::cout << parser->m3u8()->toString() << "\n";

        EXPECT_TRUE(done);

        return parser;
    }
};

TEST_F(ParserTestPlaylist, ProperHeaders) {
    FileParser* parser = getParser();
    // std::cout << parser->m3u8()->toString() << "\n";

    EXPECT_EQ(parser->m3u8()->get("version"), 4);
    EXPECT_EQ(parser->m3u8()->get("targetDuration"), 10);
    EXPECT_EQ(parser->m3u8()->get("playlistType"), "VOD");
    EXPECT_EQ(parser->m3u8()->get("mediaSequence"), 0);
    EXPECT_TRUE(parser->m3u8()->get("iframesOnly").is_object());
}

TEST_F(ParserTestPlaylist, FirstPlaylistItem) {
    FileParser* parser = getParser();

    auto item = parser->m3u8()->items[ItemTypePlaylist][0];
    EXPECT_STREQ(((std::string)item->get("title")).c_str(), "");
    EXPECT_EQ((int)item->get("duration"), 10);
    EXPECT_EQ(item->get("byteRange"), "522828@0");
    EXPECT_EQ(item->get("uri"), "hls_450k_video.ts");
}
