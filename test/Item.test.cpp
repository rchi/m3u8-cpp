#include "m3u8/items/Item.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ItemTest : public ::testing::Test, public Item {
};

TEST_F(ItemTest, Set)
{
    set("title", "hello there");
    EXPECT_EQ(propertyList["title"], "hello there");

    set("autoselect", true);
    EXPECT_TRUE(attributeList["autoselect"]);
}

TEST_F(ItemTest, Get)
{
    setProperty("uri", "/path");
    setAttribute("autoselect", true);

    // std::cerr << dump(2) << "\n";

    std::string uri = get("uri");
    EXPECT_STREQ(uri.c_str(), "/path");
    uri = propertyList["uri"];
    EXPECT_STREQ(uri.c_str(), "/path");

    EXPECT_TRUE(get("autoselect"));
    EXPECT_TRUE(attributeList["autoselect"]);
}

TEST_F(ItemTest, SetData)
{
    setData({{"autoselect", true}, {"uri", "/path"}});

    EXPECT_TRUE(get("autoselect"));
    EXPECT_EQ(get("uri"), "/path");
}

// TEST_F(ItemTest, Serialize) {
//     Item* item = createItem();
//     setData({{"autoselect", true}, {"uri", "/path"}});
//     json data = serialize();

//     EXPECT_EQ(data["attributes"], attributeList.serialize());
//     EXPECT_EQ(data["["properties"]"], (*item)["properties"]);
// }

// TEST_F(ItemTest, Unserialize) {
//     AttributeList list;
//     list.set("autoselect", true);
//     json data = {
//         {"attributes", list.serialize()},
//         {"["properties"]", {{"url", "/path"}}}
//     };
//     Item* item = Item::unserialize(data);

//     EXPECT_EQ(attributeList, list);
//     EXPECT_EQ((*item)["properties"], data["["properties"]"]);
//     EXPECT_TRUE(dynamic_cast<Item*>(item) != nullptr);
// }
