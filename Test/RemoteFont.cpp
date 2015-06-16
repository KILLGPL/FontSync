#include "../FontSync/RemoteFont.cpp"
#include <gtest/gtest.h>

TEST(RemoteFont, ComprehensiveTest)
{
	RemoteFont test("name", "category", "type", "remotefont.com/font.ttf", "0CBC6611F5540BD0809A388DC95A615B");
	ASSERT_STREQ("remotefont.com/font.ttf", test.getRemoteFile().c_str());
	ASSERT_STREQ("0CBC6611F5540BD0809A388DC95A615B", test.getMD5().c_str());
}