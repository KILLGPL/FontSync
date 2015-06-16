/// include *.hpp to appease the ODR.
#include "../FontSync/Utilities.hpp"
#include <gtest/gtest.h>

TEST(Utilities, errorString)
{
	ASSERT_NO_THROW(errorString(0));
}

TEST(Utilities, md5)
{
	/// Ref: http://onlinemd5.com/
	const std::string known_md5 = "0CBC6611F5540BD0809A388DC95A615B";
	ASSERT_STREQ(known_md5.c_str(), md5("md5_me.ttf").c_str());

	ASSERT_THROW(md5("I_DO_NOT_EXIST.ttf"), std::runtime_error);
}