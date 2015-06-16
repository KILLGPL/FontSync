#include "../FontSync/FontCache.cpp"
#include <gtest/gtest.h>
#include "../FontSync/LocalFont.cpp"
#include "../FontSync/Utilities.cpp"

//	FontCache(const std::string& fontDirectory, bool cacheImmediately = false);
TEST(FontCache, ConstructorNoCacheImmediately)
{
	FontCache* test = nullptr;
	try
	{
		ASSERT_NO_THROW(test = new FontCache("TestFonts", false));
		ASSERT_NE(nullptr, test);
		ASSERT_EQ(false, test->isInitialized());
	}
	catch (...) { /*ignore*/ }
	if (test != nullptr)
	{
		delete test;
		test = nullptr;
	}
	try
	{
		ASSERT_THROW(test = new FontCache("InvalidTestFonts$%^"), std::runtime_error);
	}
	catch (...) { /*ignore*/ }
	if (test != nullptr)
	{
		delete test;
	}
}

TEST(FontCache, ConstructorCacheImmediately)
{
	FontCache* test = nullptr;
	try
	{
		ASSERT_NO_THROW(test = new FontCache("TestFonts", true));
		ASSERT_NE(nullptr, test);
		ASSERT_EQ(true, test->isInitialized());
		ASSERT_EQ(1, test->getCachedFonts().size());
	}
	catch (...) { /*ignore*/ }
	if (test != nullptr)
	{
		delete test;
		test = nullptr;
	}

	try
	{
		ASSERT_THROW(test = new FontCache("TestFontsInvalid#$%", true), std::runtime_error);
	}
	catch (...) { /*ignore*/ }
	if (test != nullptr)
	{
		delete test;
		test = nullptr;
	}

	try
	{
		ASSERT_THROW(test = new FontCache("InvalidTestFonts", true), std::runtime_error);
	}
	catch (...) { /*ignore*/ }
	if (test != nullptr)
	{
		delete test;
		test = nullptr;
	}
}

TEST(FontCache, UpdateCache)
{
	FontCache* test = nullptr;
	try
	{
		ASSERT_NO_THROW(test = new FontCache("TestFonts", false));
		ASSERT_EQ(false, test->isInitialized());
		ASSERT_EQ(0, test->getCachedFonts().size());
		ASSERT_NO_THROW(test->updateCache());
		ASSERT_NE(nullptr, test);
		ASSERT_EQ(true, test->isInitialized());
		ASSERT_EQ(1, test->getCachedFonts().size());
	}
	catch (...) { /*ignore*/ }
	if (test != nullptr)
	{
		delete test;
		test = nullptr;
	}

	try
	{
		ASSERT_NO_THROW(test = new FontCache("InvalidTestFonts", false));
		ASSERT_THROW(test->updateCache(), std::runtime_error);
	}
	catch (...) { /*ignore*/ }
	if (test != nullptr)
	{
		delete test;
		test = nullptr;
	}
}