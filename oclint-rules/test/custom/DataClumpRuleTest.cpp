#include "TestHeaders.h"
#include "rules/custom/DataClumpRule.cpp"

TEST(DataClumpRuleTest, PropertyTest)
{
    DataClumpRule rule;
    EXPECT_EQ(3, rule.priority());
    EXPECT_EQ("data clump", rule.name());
}

TEST(DataClumpRuleTest, FirstFailingTest)
{
    EXPECT_FALSE("Start writing a new test");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
