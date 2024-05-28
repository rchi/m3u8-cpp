#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "m3u8/M3u8.h"

using json = nlohmann::json;

class M3UTest : public ::testing::Test {
protected:
    std::shared_ptr<M3u8> createM3U()
    {
        std::shared_ptr<M3u8> m3u = std::make_shared<M3u8>();
        return m3u;
    }

    // void getVariantM3U(std::function<void(std::shared_ptr<M3u8>,
    // std::exception_ptr)> callback) {
    //     std::ifstream variantFile("fixtures/variant.m3u8");
    //     if (!variantFile.is_open()) {
    //         callback(nullptr,
    //         std::make_exception_ptr(std::runtime_error("Could not open
    //         file"))); return;
    //     }

    //     try {
    //         Parser parser;
    //         std::shared_ptr<M3u8> m3u = parser.createStream(variantFile);
    //         callback(m3u, nullptr);
    //     } catch (...) {
    //         callback(nullptr, std::current_exception());
    //     }
    // }
};

TEST_F(M3UTest, Set)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    m3u->set("EXT-X-I-FRAMES-ONLY", true);
    EXPECT_TRUE(m3u->get("EXT-X-I-FRAMES-ONLY"));
}

TEST_F(M3UTest, Get)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    m3u->set("version", 4);
    EXPECT_EQ(m3u->get("version"), 4);
}

TEST_F(M3UTest, AddItem)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    size_t size = m3u->getItemCount(ItemTypePlaylist);
    EXPECT_EQ(size, 0);

    std::shared_ptr<Item> item = std::make_shared<PlaylistItem>();
    m3u->addItem(item);
    size = m3u->getItemCount(ItemTypePlaylist);
    EXPECT_EQ(size, 1);
}

TEST_F(M3UTest, AddPlaylistItem)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    m3u->addPlaylistItem(std::make_shared<PlaylistItem>());
    EXPECT_EQ((size_t)m3u->getItemCount(ItemTypePlaylist), 1);
}

TEST_F(M3UTest, RemovePlaylistItem)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    m3u->addPlaylistItem(std::make_shared<PlaylistItem>());
    m3u->removePlaylistItem(0);
    EXPECT_EQ((size_t)m3u->getItemCount(ItemTypePlaylist), 0);
}

TEST_F(M3UTest, RemovePlaylistItemOutOfRange)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    m3u->addPlaylistItem(std::make_shared<PlaylistItem>());
    m3u->addPlaylistItem(std::make_shared<PlaylistItem>());

    EXPECT_THROW(m3u->removePlaylistItem(3), std::out_of_range);
}

TEST_F(M3UTest, AddMediaItem)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    m3u->addMediaItem(std::make_shared<MediaItem>());
    EXPECT_EQ((size_t)m3u->getItemCount(ItemTypeMedia), 1);
}

TEST_F(M3UTest, AddStreamItem)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    m3u->addStreamItem(std::make_shared<StreamItem>());
    EXPECT_EQ((size_t)m3u->getItemCount(ItemTypeStream), 1);
}

TEST_F(M3UTest, AddIframeStreamItem)
{
    std::shared_ptr<M3u8> m3u = createM3U();

    m3u->addIframeStreamItem(std::make_shared<IframeStreamItem>());
    EXPECT_EQ((size_t)m3u->getItemCount(ItemTypeIframeStream), 1);
}

TEST_F(M3UTest, TotalDuration)
{
    std::shared_ptr<M3u8> m3u = createM3U();
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
    class TestableItem : public Item {
    public:
        using Item::set;
    };

    std::shared_ptr<M3u8> m3u = createM3U();

    auto set = [m3u](int index) {
        std::shared_ptr<TestableItem> item
            = std::static_pointer_cast<TestableItem>(
                m3u->getItem(ItemTypePlaylist, index));
        item->set("discontinuity", true);
    };

    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 10 }"));
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 4.5 }"));
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 45 }"));
    set(2);
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 45 }"));
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 30 }"));
    set(4);
    m3u->addPlaylistItem(
        std::make_shared<PlaylistItem>("{ \"duration\": 26 }"));

    std::vector<double> expected = {14.5, 90, 56};
    int i                        = 0;
    for (auto& duration : m3u->domainDurations()) {
        EXPECT_EQ(duration, expected[i++]);
    }
}

// TEST_F(M3UTest, Serialize) {
//     std::shared_ptr<M3u8> m3u = createM3U();
//     m3u->set("targetDuration", 10);
//     m3u->addItem(PlaylistItem, std::make_shared<PlaylistItem>());
//     json data = m3u->serialize();

//     EXPECT_EQ(data["properties"], m3u->properties);
//     EXPECT_EQ(data["items"]ItemTypePlaylist.size(),
//     m3u->getItemCount(ItemTypePlaylist));
// }

// TEST_F(M3UTest, Unserialize) {
//     json data = {
//         {"properties", {{"targetDuration", 10}}},
//         {"items", {{PlaylistItem, {{"uri", "/path"}}}}}
//     };
//     std::shared_ptr<M3u8> m3u = M3u8::unserialize(data);

//     EXPECT_EQ(m3u->properties, data["properties"]);
//     EXPECT_EQ(m3u->getItemCount(ItemTypePlaylist),
//     data["items"]ItemTypePlaylist.size());
// }

// TEST_F(M3UTest, WriteVOD) {
//     std::shared_ptr<M3u8> m3u = createM3U();
//     m3u->set("playlistType", "VOD");
//     m3u->addPlaylistItem(std::make_shared<PlaylistItem>("{ \"duration\": 1
//     }"));

//     std::string output = m3u->toString();
//     std::string endStr = "#EXT-X-ENDLIST\n";
//     std::cerr << output << "\n";
//     EXPECT_EQ(output.substr(output.size() - endStr.size()), endStr);
// }

// TEST_F(M3UTest, WriteLive) {
//     std::shared_ptr<M3u8> m3u = createM3U();
//     m3u->addPlaylistItem(std::make_shared<PlaylistItem>());

//     std::string output = m3u->toString();
//     EXPECT_EQ(output.find("#EXT-X-ENDLIST\n"), std::string::npos);
// }
