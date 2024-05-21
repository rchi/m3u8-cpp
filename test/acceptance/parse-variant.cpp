#include <gtest/gtest.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include "m3u8/FileParser.h"

extern std::string path;

using json = nlohmann::json;

class ParserTestVariant : public ::testing::Test {
protected:
    FileParser* getParser() {
        bool done         = false;
        int count         = 0;
        FileParser* parser = new FileParser;
        parser->setCallback([&](FileParser::CallbackType_t type, void*) {
            if (type == FileParser::ItemCallback) {
                count++;
            }
            else if (type == FileParser::M3u8Callback) {
                EXPECT_EQ(count, 16);
                done = true;
            }
        });

        EXPECT_EQ(parser->parseFile(path + "/variant.m3u8"), 0);

        EXPECT_TRUE(done);

        return parser;
    }
};

TEST_F(ParserTestVariant, Version) {
    FileParser* parser = getParser();

    EXPECT_EQ(parser->m3u8()->get("version"), 4);
}

TEST_F(ParserTestVariant, FirstStreamItem) {
    FileParser* parser = getParser();

    auto item = parser->m3u8()->items[ItemTypeStream][0];
    EXPECT_EQ(item->get("bandwidth"), 69334);
    EXPECT_EQ(item->get("program-id"), 1);
    EXPECT_EQ(item->get("codecs"), "avc1.42c00c");
    EXPECT_EQ(item->get("resolution")[0], 320);
    EXPECT_EQ(item->get("resolution")[1], 180);
    EXPECT_EQ(item->get("audio"), "600k");
}

TEST_F(ParserTestVariant, FirstIframeStreamItem) {
    FileParser* parser = getParser();

    auto item = parser->m3u8()->items[ItemTypeIframeStream][0];
    EXPECT_EQ(item->get("bandwidth"), 28361);
    EXPECT_EQ(item->get("uri"), "hls_64k_iframe.m3u8");
}

TEST_F(ParserTestVariant, FirstMediaItem) {
    FileParser* parser = getParser();

    auto item = parser->m3u8()->items[ItemTypeMedia][0];
    EXPECT_EQ(item->get("group-id"), "600k");
    EXPECT_EQ(item->get("language"), "eng");
    EXPECT_EQ(item->get("name"), "Audio");
    EXPECT_EQ(item->get("autoselect"), true);
    EXPECT_EQ(item->get("default"), true);
    EXPECT_EQ(item->get("uri"), "hls_600k_audio.m3u8");
    EXPECT_EQ(item->get("type"), "AUDIO");
}
