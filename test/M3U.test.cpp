#include "m3u8/M3u8.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class M3UTest : public ::testing::Test {
   protected:
    M3u8* createM3U()
    {
        M3u8* m3u = new M3u8;
        return m3u;
    }

    // void getVariantM3U(std::function<void(M3u8*, std::exception_ptr)>
    // callback) {
    //     std::ifstream variantFile("fixtures/variant.m3u8");
    //     if (!variantFile.is_open()) {
    //         callback(nullptr,
    //         std::make_exception_ptr(std::runtime_error("Could not open
    //         file"))); return;
    //     }

    //     try {
    //         Parser parser;
    //         M3u8* m3u = parser.createStream(variantFile);
    //         callback(m3u, nullptr);
    //     } catch (...) {
    //         callback(nullptr, std::current_exception());
    //     }
    // }
};

TEST_F(M3UTest, Set)
{
    M3u8* m3u = createM3U();

    m3u->set("EXT-X-I-FRAMES-ONLY", true);
    EXPECT_TRUE(m3u->get("EXT-X-I-FRAMES-ONLY"));
}

TEST_F(M3UTest, Get)
{
    M3u8* m3u = createM3U();

    m3u->set("version", 4);
    EXPECT_EQ(m3u->get("version"), 4);
}

TEST_F(M3UTest, AddItem)
{
    M3u8* m3u = createM3U();

    size_t size = m3u->items[ItemTypePlaylist].size();
    EXPECT_EQ(size, 0);

    std::shared_ptr<Item> item = std::make_shared<PlaylistItem>();
    m3u->addItem(item);
    size = m3u->items[ItemTypePlaylist].size();
    EXPECT_EQ(size, 1);
}

TEST_F(M3UTest, AddPlaylistItem)
{
    M3u8* m3u = createM3U();

    m3u->addPlaylistItem(std::make_shared<PlaylistItem>());
    EXPECT_EQ((size_t)m3u->items[ItemTypePlaylist].size(), 1);
}

TEST_F(M3UTest, RemovePlaylistItem)
{
    M3u8* m3u = createM3U();

    m3u->addPlaylistItem(std::make_shared<PlaylistItem>());
    m3u->removePlaylistItem(0);
    EXPECT_EQ((size_t)m3u->items[ItemTypePlaylist].size(), 0);
}

TEST_F(M3UTest, RemovePlaylistItemOutOfRange)
{
    M3u8* m3u = createM3U();

    m3u->addPlaylistItem(std::make_shared<PlaylistItem>());
    m3u->addPlaylistItem(std::make_shared<PlaylistItem>());

    EXPECT_THROW(m3u->removePlaylistItem(3), std::out_of_range);
}

TEST_F(M3UTest, AddMediaItem)
{
    M3u8* m3u = createM3U();

    m3u->addMediaItem(std::make_shared<MediaItem>());
    EXPECT_EQ((size_t)m3u->items[ItemTypeMedia].size(), 1);
}

TEST_F(M3UTest, AddStreamItem)
{
    M3u8* m3u = createM3U();

    m3u->addStreamItem(std::make_shared<StreamItem>());
    EXPECT_EQ((size_t)m3u->items[ItemTypeStream].size(), 1);
}

TEST_F(M3UTest, AddIframeStreamItem)
{
    M3u8* m3u = createM3U();

    m3u->addIframeStreamItem(std::make_shared<IframeStreamItem>());
    EXPECT_EQ((size_t)m3u->items[ItemTypeIframeStream].size(), 1);
}

TEST_F(M3UTest, TotalDuration)
{
    M3u8* m3u = createM3U();
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 10 }"));
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 4.5 }"));
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 45 }"));
    EXPECT_EQ(m3u->totalDuration(), 59.5);
}

TEST_F(M3UTest, DomainDurations)
{
    M3u8* m3u = createM3U();

    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 10 }"));
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 4.5 }"));
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 45 }"));
    m3u->getItem(ItemTypePlaylist, 2)->set("discontinuity", true);
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 45 }"));
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 30 }"));
    m3u->getItem(ItemTypePlaylist, 4)->set("discontinuity", true);
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 26 }"));

    std::vector<double> expected = {14.5, 90, 56};
    int i                        = 0;
    for (auto& duration : m3u->domainDurations()) {
        EXPECT_EQ(duration, expected[i++]);
    }
}

TEST_F(M3UTest, Merge)
{
    M3u8* m3u1 = createM3U();
    m3u1->addPlaylistItem(std::make_shared<PlaylistItem>());
    m3u1->addPlaylistItem(std::make_shared<PlaylistItem>());
    m3u1->addPlaylistItem(std::make_shared<PlaylistItem>());

    M3u8* m3u2 = createM3U();
    m3u2->addPlaylistItem(std::make_shared<PlaylistItem>());
    m3u2->addPlaylistItem(std::make_shared<PlaylistItem>());

    m3u1->merge(m3u2);
    EXPECT_TRUE((bool)m3u1->getItem(ItemTypePlaylist, 3)->get("discontinuity"));
    int discontinuityCount = 0;
    for (auto& item : m3u1->items[ItemTypePlaylist]) {
        if (item->propertyList["discontinuity"].is_boolean()) {
            if ((bool)(item->propertyList["discontinuity"])) {
                discontinuityCount++;
            }
        }
    }
    EXPECT_EQ(discontinuityCount, 1);
}

TEST_F(M3UTest, MergeTargetDuration)
{
    M3u8* m3u1 = createM3U();
    m3u1->set("targetDuration", 10);
    m3u1->addPlaylistItem(std::make_shared<PlaylistItem>());

    M3u8* m3u2 = createM3U();
    m3u2->set("targetDuration", 11);
    m3u2->addPlaylistItem(std::make_shared<PlaylistItem>());

    m3u1->merge(m3u2);
    EXPECT_EQ(m3u1->get("targetDuration"), 11);
}

// TEST_F(M3UTest, Serialize) {
//     M3u8* m3u = createM3U();
//     m3u->set("targetDuration", 10);
//     m3u->addItem(PlaylistItem, std::make_shared<PlaylistItem>());
//     json data = m3u->serialize();

//     EXPECT_EQ(data["properties"], m3u->properties);
//     EXPECT_EQ(data["items"]ItemTypePlaylist.size(),
//     m3u->items[ItemTypePlaylist].size());
// }

// TEST_F(M3UTest, Unserialize) {
//     json data = {
//         {"properties", {{"targetDuration", 10}}},
//         {"items", {{PlaylistItem, {{"uri", "/path"}}}}}
//     };
//     M3u8* m3u = M3u8::unserialize(data);

//     EXPECT_EQ(m3u->properties, data["properties"]);
//     EXPECT_EQ(m3u->items[ItemTypePlaylist].size(),
//     data["items"]ItemTypePlaylist.size());
// }

// TEST_F(M3UTest, WriteVOD) {
//     M3u8* m3u = createM3U();
//     m3u->set("playlistType", "VOD");
//     m3u->addPlaylistItem(std::make_shared<PlaylistItem>("{ \"duration\": 1
//     }"));

//     std::string output = m3u->toString();
//     std::string endStr = "#EXT-X-ENDLIST\n";
//     std::cerr << output << "\n";
//     EXPECT_EQ(output.substr(output.size() - endStr.size()), endStr);
// }

// TEST_F(M3UTest, WriteLive) {
//     M3u8* m3u = createM3U();
//     m3u->addPlaylistItem(std::make_shared<PlaylistItem>());

//     std::string output = m3u->toString();
//     EXPECT_EQ(output.find("#EXT-X-ENDLIST\n"), std::string::npos);
// }
