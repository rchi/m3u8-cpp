#include <gtest/gtest.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "m3u8/FileParser.h"

extern std::string path;

using json = nlohmann::json;

class ParserTestIFrame : public ::testing::Test {
protected:
    FileParser* getParser()
    {
        bool done          = false;
        int count          = 0;
        FileParser* parser = new FileParser;
        parser->setCallback(
            [&](FileParser::CallbackType_t type, std::shared_ptr<void>) {
                if (type == FileParser::ItemCallback) {
                    count++;
                }
                else if (type == FileParser::M3u8Callback) {
                    EXPECT_EQ(count, 36);
                    done = true;
                }
            });

        EXPECT_EQ(parser->parseFile(path + "/iframe.m3u8"), 0);

        EXPECT_TRUE(done);

        return parser;
    }
};

TEST_F(ParserTestIFrame, ProperHeaders)
{
    FileParser* parser = getParser();
    // std::cout << parser->m3u8()->toString() << "\n";

    EXPECT_EQ(parser->m3u8()->get("version"), 4);
    EXPECT_EQ(parser->m3u8()->get("targetDuration"), 10);
    EXPECT_EQ(parser->m3u8()->get("playlistType"), "VOD");
    EXPECT_EQ(parser->m3u8()->get("mediaSequence"), 0);
    EXPECT_EQ(parser->m3u8()->get("iframesOnly"), true);
}

TEST_F(ParserTestIFrame, FirstIframeStreamItem)
{
    FileParser* parser = getParser();
    auto item          = parser->m3u8()->getItem(ItemTypePlaylist, 0);
    EXPECT_NE(item, nullptr);
    if (item != NULL) {
        EXPECT_EQ(item->get("title"), "");
        EXPECT_EQ(item->get("duration"), 5);
        EXPECT_EQ(item->get("byteRange"), "376@940");
        EXPECT_EQ(item->get("uri"), "hls_1500k_video.ts");
    }
}
