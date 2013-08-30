#include "TestHeaders.h"

#include "rules/custom/FeatureEnvyRule.cpp"

TEST(FeatureEnvyRuleTest, PropertyTest)
{
    FeatureEnvyRule rule;
    EXPECT_EQ(3, rule.priority());
    EXPECT_EQ("feature envy", rule.name());
}

TEST(FeatureEnvyRuleTest, ObjCImplementationNoMethod)
{
  testRuleOnObjCCode(new FeatureEnvyRule(), "@interface a\n@end\n@implementation a\n@end\n");
}

TEST(FeatureEnvyRuleTest, ObjcMethodMessageOther)
{
    testRuleOnObjCCode(new FeatureEnvyRule(),
      "@interface B \n"
      "@end \n"

      "@implementation B \n"
      "- (void) f { \n"
      "} \n"
      "@end \n"

      "@interface A \n"
      "@end \n"

      "@implementation A \n"
      "- (void)f { \n"
      "    B *b; \n"
      "    [b f]; \n"
      "} \n"
      "@end\n",
      0, 10, 1, 13, 1, "Method f messages B more than self.");
}

TEST(FeatureEnvyRuleTest, ObjcMethodMessageSelf)
{
    testRuleOnObjCCode(new FeatureEnvyRule(),
      "@interface B \n"
      "@end \n"

      "@implementation B \n"
      "- (void) f { \n"
      "} \n"
      "@end \n"

      "@interface A \n"
      "@end \n"

      "@implementation A \n"
      "- (void)f { \n"
      "    [self f]; \n"
      "} \n"
      "@end\n");
}

TEST(FeatureEnvyRuleTest, ObjcMethodMessageSelfAndOtherEqually)
{
    testRuleOnObjCCode(new FeatureEnvyRule(),
      "@interface B \n"
      "@end \n"

      "@implementation B \n"
      "- (void) f { \n"
      "} \n"
      "@end \n"

      "@interface A \n"
      "@end \n"

      "@implementation A \n"
      "- (void)f { \n"
      "    B *b; \n"
      "    [b f]; \n"
      "    [self f]; \n"
      "} \n"
      "@end\n");
}

TEST(FeatureEnvyRuleTest, ObjcMethodMessageSelfLessThanOther)
{
    testRuleOnObjCCode(new FeatureEnvyRule(),
      "@interface B \n"
      "@end \n"

      "@implementation B \n"
      "- (void) f { \n"
      "} \n"
      "@end \n"

      "@interface A \n"
      "@end \n"

      "@implementation A \n"
      "- (void)f { \n"
      "    B *b; \n"
      "    [b f]; \n"
      "    [self f]; \n"
      "    [b f]; \n"
      "} \n"
      "@end\n",
      0, 10, 1, 15, 1, "Method f messages B more than self.");
}

TEST(FeatureEnvyRuleTest, ObjcMethodAccessesPropertyOfAnotherClass)
{
    testRuleOnObjCCode(new FeatureEnvyRule(),
      "@interface B \n"
      "@property int p; \n"
      "@end \n"

      "@implementation B \n"
      "@end \n"

      "@interface A \n"
      "@end \n"

      "@implementation A \n"
      "- (void)f { \n"
      "    B *b; \n"
      "    b.p = 5; \n"
      "} \n"
      "@end\n",
      0, 9, 1, 12, 1, "Method f messages B more than self.");
}

TEST(FeatureEnvyRuleTest, ObjcMethodAccessesImplicitPropertySetterOfAnotherClass)
{
    testRuleOnObjCCode(new FeatureEnvyRule(),
      "@interface B \n"
      "- (int)p; \n"
      "- (void)setP:(int)value; \n"
      "@end \n"

      "@implementation B \n"
      "@end \n"

      "@interface A \n"
      "@end \n"

      "@implementation A \n"
      "- (void)f { \n"
      "    B *b; \n"
      "    b.p = 5; \n"
      "} \n"
      "@end\n",
      0, 10, 1, 13, 1, "Method f messages B more than self.");
}

TEST(FeatureEnvyRuleTest, ObjcMethodAccessesImplicitPropertyGetterOfAnotherClass)
{
    testRuleOnObjCCode(new FeatureEnvyRule(),
      "@interface B \n"
      "- (int)p; \n"
      "- (void)setP:(int)value; \n"
      "@end \n"

      "@implementation B \n"
      "@end \n"

      "@interface A \n"
      "@end \n"

      "@implementation A \n"
      "- (int)f { \n"
      "    B *b; \n"
      "    return b.p; \n"
      "} \n"
      "@end\n",
      0, 10, 1, 13, 1, "Method f messages B more than self.");
}

TEST(FeatureEnvyRuleTest, ObjcMethodAccessesIvarOfAnotherClass)
{
    testRuleOnObjCCode(new FeatureEnvyRule(),
      "@interface B {\n"
      "    @public \n"
      "    int _p; \n"
      "}; \n"
      "@end \n"

      "@implementation B \n"
      "@end \n"

      "@interface A \n"
      "@end \n"

      "@implementation A \n"
      "- (void)f { \n"
      "    B *b; \n"
      "    b->_p = 5; \n"
      "} \n"
      "@end\n",
      0, 11, 1, 14, 1, "Method f messages B more than self.");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}