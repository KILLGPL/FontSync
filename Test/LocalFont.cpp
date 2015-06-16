#include "../FontSync/LocalFont.hpp"
#include <gtest/gtest.h>

TEST(LocalFont, ComprehensiveTest)
{
	const std::string known_md5 = "0CBC6611F5540BD0809A388DC95A615B";
	LocalFont test("name", "category", "type", "md5_me.ttf");
	ASSERT_EQ("md5_me.ttf", test.getLocalFile());
	ASSERT_STREQ(known_md5.c_str(), test.getMD5().c_str());
}