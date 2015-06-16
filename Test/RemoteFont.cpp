#include "../FontSync/RemoteFont.cpp"
#include <gtest/gtest.h>

TEST(RemoteFont, ComprehensiveTest)
{
	RemoteFont test("name", "category", "type", "0CBC6611F5540BD0809A388DC95A615B");
	ASSERT_STREQ("0CBC6611F5540BD0809A388DC95A615B", test.getMD5().c_str());
}