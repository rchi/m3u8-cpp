
#include "m3u8/Parser.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ParserTest : public ::testing::Test {
   protected:
    M3UParser* getParser()
    {
        M3UParser* parser = new M3UParser;
        return parser;
    }
};

TEST_F(ParserTest, ParseLine)
{
    M3UParser* parser = getParser();

    // should call known tags
    parser->parseLine("#EXT-X-MEDIA:GROUP-ID=\"600k\", LANGUAGE=\"eng\"");
    // std::cout << parser->m3u8()->toString() << "\n";
    EXPECT_STREQ(((std::string)parser->m3u8()
                      ->getItem(ItemTypeMedia, 0)
                      ->get("GROUP-ID"))
                     .c_str(),
                 "600k");
    EXPECT_STREQ(((std::string)parser->m3u8()
                      ->getItem(ItemTypeMedia, 0)
                      ->get("LANGUAGE"))
                     .c_str(),
                 "eng");

    // should set data on m3u on unknown tags
    parser->parseLine("#THIS-IS-A-TAG:some value");
    // std::cout << parser->m3u8()->toString() << "\n";
    EXPECT_STREQ(((std::string)parser->m3u8()->get("THIS-IS-A-TAG")).c_str(),
                 "some value");

    // should split on first colon only
    parser->parseLine("#THIS-IS-A-TAG:http://www.ted.com");
    // std::cout << parser->m3u8()->toString() << "\n";
    EXPECT_STREQ(((std::string)parser->m3u8()->get("THIS-IS-A-TAG")).c_str(),
                 "http://www.ted.com");
}

// TEST_F(ParserTest, AddItem) {
//     M3UParser* parser = getParser();

//     PlaylistItem* item = new PlaylistItem;
//     parser->addItem(item);
//     EXPECT_EQ(parser->currentItem, item);
// }

TEST_F(ParserTest, EXTINF)
{
    M3UParser* parser = getParser();

    parser->parseLine("#EXTINF:4.5,some title");
    EXPECT_EQ((double)parser->getCurrentItem()->get("duration"), 4.5);
    EXPECT_STREQ(((std::string)parser->getCurrentItem()->get("title")).c_str(),
                 "some title");
}

TEST_F(ParserTest, EXT_X_BYTERANGE)
{
    M3UParser* parser = getParser();

    parser->parseLine("#EXTINF:4.5,");
    parser->parseLine("#EXT-X-BYTERANGE:45@90");
    EXPECT_STREQ(
        ((std::string)parser->getCurrentItem()->get("byteRange")).c_str(),
        "45@90");
}

TEST_F(ParserTest, EXT_X_DISCONTINUITY)
{
    M3UParser* parser = getParser();
    parser->parseLine("#EXT-X-DISCONTINUITY");
    parser->parseLine("#EXTINF:4.5,some title");
    // std::cout << parser->getCurrentItem()->dump() << "\n";
    EXPECT_EQ(parser->getCurrentItem()->itemType(), ItemTypePlaylist);
    EXPECT_EQ((double)parser->getCurrentItem()->get("duration"), 4.5);
    EXPECT_STREQ(((std::string)parser->getCurrentItem()->get("title")).c_str(),
                 "some title");
    EXPECT_EQ((bool)parser->getCurrentItem()->get("discontinuity"), true);
}

TEST_F(ParserTest, EXT_X_STREAM_INF)
{
    M3UParser* parser = getParser();

    parser->parseLine("#EXT-X-STREAM-INF:NAME=\"I am a stream!\"");
    std::string name = parser->m3u8()->getItem(ItemTypeStream, 0)->get("name");
    // std::cout << parser->m3u8()->toString() << "\n";
    EXPECT_STREQ(name.c_str(), "I am a stream!");
}

TEST_F(ParserTest, EXT_X_I_FRAME_STREAM_INF)
{
    M3UParser* parser = getParser();

    parser->parseLine(
        "#EXT-X-I-FRAME-STREAM-INF:NAME=\"I am an iframe stream!\"");
    EXPECT_STREQ(((std::string)parser->getCurrentItem()->get("name")).c_str(),
                 "I am an iframe stream!");
}

TEST_F(ParserTest, EXT_X_MEDIA)
{
    M3UParser* parser = getParser();

    parser->parseLine("#EXT-X-MEDIA:NAME=\"I am a media item!\"");
    EXPECT_EQ(parser->getCurrentItem()->itemType(), ItemTypeMedia);
    // std::cout << parser->getCurrentItem()->dump(4) << "\n";
    EXPECT_STREQ(((std::string)parser->getCurrentItem()->get("name")).c_str(),
                 "I am a media item!");
}

TEST_F(ParserTest, ParseAttributes)
{
    M3UParser* parser = getParser();

    json keyValues = parser->parseAttributes(
        "KEY=\"I, am a value\",RESOLUTION=640x360,FORCED=NO");
    EXPECT_EQ(keyValues["KEY"].is_string(), true);
    EXPECT_STREQ(((std::string)keyValues["KEY"]).c_str(), "I, am a value");
    EXPECT_STREQ(((std::string)keyValues["FORCED"]).c_str(), "NO");
}
