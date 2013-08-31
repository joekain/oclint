#include "TestHeaders.h"
#include "rules/custom/DataClumpRule.cpp"

TEST(DataClumpRuleTest, PropertyTest)
{
    DataClumpRule rule;
    EXPECT_EQ(3, rule.priority());
    EXPECT_EQ("data clump", rule.name());
}

TEST(DataClumpRuleTest, SingleFunction)
{
    testRuleOnCode(new DataClumpRule(),
        "void m(int i, int j) {  }");
}

TEST(DataClumpRuleTest, ThreeFunctionsWithClump)
{
    testRuleOnCode(new DataClumpRule(),
        "void func1(int i, int j) {  } \n"
        "void func2(int i, int j) {  } \n"
        "void func3(int i, int j) {  } \n",
        0, 1, 19, 1, 23, "Data clump: Parameters ( j i ) used together in 3 methods.");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
