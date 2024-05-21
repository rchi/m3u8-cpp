#include "m3u8/items/AttributeList.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class AttributeListTest : public ::testing::Test {
   protected:
    AttributeList* createAttributeList()
    {
        AttributeList* list = new AttributeList;
        list->set("bandwidth", 1);
        (*list)["audio"] = "hello";
        return list;
    }
};

TEST_F(AttributeListTest, Set)
{
    AttributeList* list = createAttributeList();

    EXPECT_EQ((*list)["bandwidth"], 1);
}

TEST_F(AttributeListTest, Get)
{
    AttributeList* list = createAttributeList();

    EXPECT_EQ(list->get("bandwidth"), 1);
}

TEST_F(AttributeListTest, GetCoerced)
{
    AttributeList* list = createAttributeList();

    std::string value = (*list)["audio"];
    EXPECT_STREQ(value.c_str(), "hello");
}
