#include "m3u8/items/Item.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ItemTest : public ::testing::Test {
   protected:
    Item* createItem()
    {
        Item* item = new Item;
        return item;
    }
};

TEST_F(ItemTest, Set)
{
    Item* item = createItem();

    item->set("title", "hello there");
    EXPECT_EQ((*item).propertyList["title"], "hello there");

    item->set("autoselect", true);
    EXPECT_TRUE(item->attributeList["autoselect"]);
}

TEST_F(ItemTest, Get)
{
    Item* item = createItem();

    item->setProperty("uri", "/path");
    item->setAttribute("autoselect", true);

    // std::cerr << item->dump(2) << "\n";

    std::string uri = item->getProperty("uri");
    EXPECT_STREQ(uri.c_str(), "/path");
    uri = (*item).propertyList["uri"];
    EXPECT_STREQ(uri.c_str(), "/path");

    EXPECT_TRUE(item->attribute("autoselect"));
    EXPECT_TRUE(item->attributeList["autoselect"]);
}

TEST_F(ItemTest, SetData)
{
    Item* item = createItem();
    item->setData({{"autoselect", true}, {"uri", "/path"}});

    EXPECT_TRUE(item->get("autoselect"));
    EXPECT_EQ(item->get("uri"), "/path");
}

// TEST_F(ItemTest, Serialize) {
//     Item* item = createItem();
//     item->setData({{"autoselect", true}, {"uri", "/path"}});
//     json data = item->serialize();

//     EXPECT_EQ(data["attributes"], item->attributeList.serialize());
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

//     EXPECT_EQ(item->attributeList, list);
//     EXPECT_EQ((*item)["properties"], data["["properties"]"]);
//     EXPECT_TRUE(dynamic_cast<Item*>(item) != nullptr);
// }
