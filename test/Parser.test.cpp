
#include "m3u8/Parser.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ParserTest : public ::testing::Test, public M3UParser {
protected:
    M3UParser* getParser()
    {
        M3UParser* parser = new M3UParser;
        return parser;
    }
};

TEST_F(ParserTest, ParseLine)
{
    // should call known tags
    parseLine("#EXT-X-MEDIA:GROUP-ID=\"600k\", LANGUAGE=\"eng\"");
    // std::cout << m3u8()->toString() << "\n";
    EXPECT_STREQ(
        ((std::string)m3u8()->getItem(ItemTypeMedia, 0)->get("GROUP-ID"))
            .c_str(),
        "600k");
    EXPECT_STREQ(
        ((std::string)m3u8()->getItem(ItemTypeMedia, 0)->get("LANGUAGE"))
            .c_str(),
        "eng");

    // should set data on m3u on unknown tags
    parseLine("#THIS-IS-A-TAG:some value");
    // std::cout << m3u8()->toString() << "\n";
    EXPECT_STREQ(((std::string)m3u8()->get("THIS-IS-A-TAG")).c_str(),
                 "some value");

    // should split on first colon only
    parseLine("#THIS-IS-A-TAG:http://www.ted.com");
    // std::cout << m3u8()->toString() << "\n";
    EXPECT_STREQ(((std::string)m3u8()->get("THIS-IS-A-TAG")).c_str(),
                 "http://www.ted.com");
}

// TEST_F(ParserTest, AddItem) {
//     PlaylistItem* item = new PlaylistItem;
//     addItem(item);
//     EXPECT_EQ(currentItem, item);
// }

TEST_F(ParserTest, EXTINF)
{
    parseLine("#EXTINF:4.5,some title");
    EXPECT_EQ((double)getCurrentItem()->get("duration"), 4.5);
    EXPECT_STREQ(((std::string)getCurrentItem()->get("title")).c_str(),
                 "some title");
}

TEST_F(ParserTest, EXT_X_BYTERANGE)
{
    parseLine("#EXTINF:4.5,");
    parseLine("#EXT-X-BYTERANGE:45@90");
    EXPECT_STREQ(((std::string)getCurrentItem()->get("byteRange")).c_str(),
                 "45@90");
}

TEST_F(ParserTest, EXT_X_DISCONTINUITY)
{
    parseLine("#EXT-X-DISCONTINUITY");
    parseLine("#EXTINF:4.5,some title");
    // std::cout << getCurrentItem()->dump() << "\n";
    EXPECT_EQ(getCurrentItem()->itemType(), ItemTypePlaylist);
    EXPECT_EQ((double)getCurrentItem()->get("duration"), 4.5);
    EXPECT_STREQ(((std::string)getCurrentItem()->get("title")).c_str(),
                 "some title");
    EXPECT_EQ((bool)getCurrentItem()->get("discontinuity"), true);
}

TEST_F(ParserTest, EXT_X_STREAM_INF)
{
    parseLine("#EXT-X-STREAM-INF:NAME=\"I am a stream!\"");
    std::string name = m3u8()->getItem(ItemTypeStream, 0)->get("name");
    // std::cout << m3u8()->toString() << "\n";
    EXPECT_STREQ(name.c_str(), "I am a stream!");
}

TEST_F(ParserTest, EXT_X_I_FRAME_STREAM_INF)
{
    parseLine("#EXT-X-I-FRAME-STREAM-INF:NAME=\"I am an iframe stream!\"");
    EXPECT_STREQ(((std::string)getCurrentItem()->get("name")).c_str(),
                 "I am an iframe stream!");
}

TEST_F(ParserTest, EXT_X_MEDIA)
{
    parseLine("#EXT-X-MEDIA:NAME=\"I am a media item!\"");
    EXPECT_EQ(getCurrentItem()->itemType(), ItemTypeMedia);
    // std::cout << getCurrentItem()->dump(4) << "\n";
    EXPECT_STREQ(((std::string)getCurrentItem()->get("name")).c_str(),
                 "I am a media item!");
}

TEST_F(ParserTest, ParseAttributes)
{
    json keyValues
        = parseAttributes("KEY=\"I, am a value\",RESOLUTION=640x360,FORCED=NO");
    EXPECT_EQ(keyValues["KEY"].is_string(), true);
    EXPECT_STREQ(((std::string)keyValues["KEY"]).c_str(), "I, am a value");
    EXPECT_STREQ(((std::string)keyValues["FORCED"]).c_str(), "NO");
}

TEST_F(ParserTest, Merge)
{
    class TestableItem : public Item {
    public:
        using Item::set;
    };

    auto set = [](M3UParser* parser, std::string uri) {
        std::shared_ptr<TestableItem> item
            = std::static_pointer_cast<TestableItem>(
                parser->m3u8()->getLastItem(ItemTypePlaylist));
        item->set("uri", uri);
    };

    M3UParser* parser1 = getParser();
    parser1->m3u8()->addPlaylistItem(std::make_shared<PlaylistItem>());
    set(parser1, "uri_1.ts");
    parser1->m3u8()->addPlaylistItem(std::make_shared<PlaylistItem>());
    set(parser1, "uri_2.ts");

    M3UParser* parser2 = getParser();
    parser2->m3u8()->addPlaylistItem(std::make_shared<PlaylistItem>());
    set(parser2, "uri_1.ts");
    parser2->m3u8()->addPlaylistItem(std::make_shared<PlaylistItem>());
    set(parser2, "uri_2.ts");
    parser2->m3u8()->addPlaylistItem(std::make_shared<PlaylistItem>());
    set(parser2, "uri_3.ts");

    parser1->merge(parser2->m3u8());
    EXPECT_EQ((size_t)parser1->m3u8()->getItemCount(ItemTypePlaylist), 3);
}

TEST_F(ParserTest, MergeTargetDuration)
{
    M3UParser* parser1 = getParser();
    parser1->m3u8()->set("targetDuration", 10);
    parser1->m3u8()->addPlaylistItem(std::make_shared<PlaylistItem>());

    M3UParser* parser2 = getParser();
    parser2->m3u8()->set("targetDuration", 11);
    parser2->m3u8()->addPlaylistItem(std::make_shared<PlaylistItem>());

    parser1->merge(parser2->m3u8());
    EXPECT_EQ(parser1->m3u8()->get("targetDuration"), 11);
}
