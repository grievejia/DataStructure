#include "DataStructure/StringView.h"

#include "gtest/gtest.h"

using namespace ds;

namespace {

TEST(StringViewTest, Construction) {
    EXPECT_EQ("", StringView());
    EXPECT_EQ("hello", StringView("hello"));
    EXPECT_EQ("hello", StringView("hello world", 5));
    EXPECT_EQ("hello", StringView(std::string("hello")));
}

TEST(StringViewTest, Iteration) {
    StringView S("hello");
    const char* p = "hello";
    for (const char *it = S.begin(), *ie = S.end(); it != ie; ++it, ++p)
        EXPECT_EQ(*it, *p);
}

TEST(StringViewTest, StringOps) {
    const char* p = "hello";
    EXPECT_EQ(p, StringView(p, 0).data());
    EXPECT_TRUE(StringView().empty());
    EXPECT_EQ((size_t)5, StringView("hello").size());
    EXPECT_EQ(-1, StringView("aab").compare("aad"));
    EXPECT_EQ(0, StringView("aab").compare("aab"));
    EXPECT_EQ(1, StringView("aab").compare("aaa"));
    EXPECT_EQ(-1, StringView("aab").compare("aabb"));
    EXPECT_EQ(1, StringView("aab").compare("aa"));
    EXPECT_EQ(1, StringView("\xFF").compare("\1"));

    EXPECT_EQ(-1, StringView("AaB").compare_lower("aAd"));
    EXPECT_EQ(0, StringView("AaB").compare_lower("aab"));
    EXPECT_EQ(1, StringView("AaB").compare_lower("AAA"));
    EXPECT_EQ(-1, StringView("AaB").compare_lower("aaBb"));
    EXPECT_EQ(-1, StringView("AaB").compare_lower("bb"));
    EXPECT_EQ(1, StringView("aaBb").compare_lower("AaB"));
    EXPECT_EQ(1, StringView("bb").compare_lower("AaB"));
    EXPECT_EQ(1, StringView("AaB").compare_lower("aA"));
    EXPECT_EQ(1, StringView("\xFF").compare_lower("\1"));
}

TEST(StringViewTest, Operators) {
    EXPECT_EQ("", StringView());
    EXPECT_TRUE(StringView("aab") < StringView("aad"));
    EXPECT_FALSE(StringView("aab") < StringView("aab"));
    EXPECT_TRUE(StringView("aab") <= StringView("aab"));
    EXPECT_FALSE(StringView("aab") <= StringView("aaa"));
    EXPECT_TRUE(StringView("aad") > StringView("aab"));
    EXPECT_FALSE(StringView("aab") > StringView("aab"));
    EXPECT_TRUE(StringView("aab") >= StringView("aab"));
    EXPECT_FALSE(StringView("aaa") >= StringView("aab"));
    EXPECT_EQ(StringView("aab"), StringView("aab"));
    EXPECT_FALSE(StringView("aab") == StringView("aac"));
    EXPECT_FALSE(StringView("aab") != StringView("aab"));
    EXPECT_TRUE(StringView("aab") != StringView("aac"));
    EXPECT_EQ('a', StringView("aab")[1]);
}

TEST(StringViewTest, Substr) {
    StringView Str("hello");
    EXPECT_EQ("lo", Str.substr(3));
    EXPECT_EQ("", Str.substr(100));
    EXPECT_EQ("hello", Str.substr(0, 100));
    EXPECT_EQ("o", Str.substr(4, 10));
}

TEST(StringViewTest, StartsWith) {
    StringView Str("hello");
    EXPECT_TRUE(Str.startswith(""));
    EXPECT_TRUE(Str.startswith("he"));
    EXPECT_FALSE(Str.startswith("helloworld"));
    EXPECT_FALSE(Str.startswith("hi"));
}

TEST(StringViewTest, EndsWith) {
    StringView Str("hello");
    EXPECT_TRUE(Str.endswith(""));
    EXPECT_TRUE(Str.endswith("lo"));
    EXPECT_FALSE(Str.endswith("helloworld"));
    EXPECT_FALSE(Str.endswith("worldhello"));
    EXPECT_FALSE(Str.endswith("so"));
}

TEST(StringViewTest, Trim) {
    StringView Str0("hello");
    StringView Str1(" hello ");
    StringView Str2("  hello  ");

    EXPECT_EQ(StringView("hello"), Str0.rtrim());
    EXPECT_EQ(StringView(" hello"), Str1.rtrim());
    EXPECT_EQ(StringView("  hello"), Str2.rtrim());
    EXPECT_EQ(StringView("hello"), Str0.ltrim());
    EXPECT_EQ(StringView("hello "), Str1.ltrim());
    EXPECT_EQ(StringView("hello  "), Str2.ltrim());
    EXPECT_EQ(StringView("hello"), Str0.trim());
    EXPECT_EQ(StringView("hello"), Str1.trim());
    EXPECT_EQ(StringView("hello"), Str2.trim());

    EXPECT_EQ(StringView("ello"), Str0.trim("hhhhhhhhhhh"));

    EXPECT_EQ(StringView(""), StringView("").trim());
    EXPECT_EQ(StringView(""), StringView(" ").trim());
    EXPECT_EQ(StringView("\0", 1), StringView(" \0 ", 3).trim());
    EXPECT_EQ(StringView("\0\0", 2), StringView("\0\0", 2).trim());
    EXPECT_EQ(StringView("x"),
              StringView("\0\0x\0\0", 5).trim(StringView("\0", 1)));
}

TEST(StringViewTest, Find) {
    StringView Str("hello");
    EXPECT_EQ(2U, Str.find('l'));
    EXPECT_EQ(StringView::npos, Str.find('z'));
    EXPECT_EQ(StringView::npos, Str.find("helloworld"));
    EXPECT_EQ(0U, Str.find("hello"));
    EXPECT_EQ(1U, Str.find("ello"));
    EXPECT_EQ(StringView::npos, Str.find("zz"));
    EXPECT_EQ(2U, Str.find("ll", 2));
    EXPECT_EQ(StringView::npos, Str.find("ll", 3));
    EXPECT_EQ(0U, Str.find(""));
    StringView LongStr("hellx xello hell ello world foo bar hello");
    EXPECT_EQ(36U, LongStr.find("hello"));
    EXPECT_EQ(28U, LongStr.find("foo"));
    EXPECT_EQ(12U, LongStr.find("hell", 2));
    EXPECT_EQ(0U, LongStr.find(""));

    EXPECT_EQ(3U, Str.rfind('l'));
    EXPECT_EQ(StringView::npos, Str.rfind('z'));
    EXPECT_EQ(StringView::npos, Str.rfind("helloworld"));
    EXPECT_EQ(0U, Str.rfind("hello"));
    EXPECT_EQ(1U, Str.rfind("ello"));
    EXPECT_EQ(StringView::npos, Str.rfind("zz"));

    EXPECT_EQ(2U, Str.find_first_of('l'));
    EXPECT_EQ(1U, Str.find_first_of("el"));
    EXPECT_EQ(StringView::npos, Str.find_first_of("xyz"));

    EXPECT_EQ(1U, Str.find_first_not_of('h'));
    EXPECT_EQ(4U, Str.find_first_not_of("hel"));
    EXPECT_EQ(StringView::npos, Str.find_first_not_of("hello"));

    EXPECT_EQ(3U, Str.find_last_not_of('o'));
    EXPECT_EQ(1U, Str.find_last_not_of("lo"));
    EXPECT_EQ(StringView::npos, Str.find_last_not_of("helo"));
}

TEST(StringViewTest, Count) {
    StringView Str("hello");
    EXPECT_EQ(2U, Str.count('l'));
    EXPECT_EQ(1U, Str.count('o'));
    EXPECT_EQ(0U, Str.count('z'));
    EXPECT_EQ(0U, Str.count("helloworld"));
    EXPECT_EQ(1U, Str.count("hello"));
    EXPECT_EQ(1U, Str.count("ello"));
    EXPECT_EQ(0U, Str.count("zz"));
}
}