#include "../FontSync/FontBase.cpp"
#include <gtest/gtest.h>

class TestFontBase : public FontBase
{
	std::string dummy;
public:
	TestFontBase(const std::string& name,
		const std::string& category,
		const std::string& type) : FontBase(name, category, type)
	{

	}

	/// N/A
	const std::string& getMD5() const
	{
		return dummy;
	}
};

TEST(FontBase, Constructor)
{
	FontBase* test = nullptr;
	try
	{
		ASSERT_NO_THROW(test = new TestFontBase("name", "category", "type"));
	}
	catch (...) { /*ignore*/ }
	ASSERT_NE(nullptr, test);
	ASSERT_STREQ("name", test->getName().c_str());
	ASSERT_STREQ("category", test->getCategory().c_str());
	ASSERT_STREQ("type", test->getType().c_str());

	if (test != nullptr)
	{
		delete test;
	}
}

TEST(FontBase, CopyConstructor)
{
	TestFontBase source("name", "category", "type");
	FontBase* test = nullptr;
	try
	{
		ASSERT_NO_THROW(test = new TestFontBase(source));
	}
	catch (...) { /*ignore*/ }
	ASSERT_NE(nullptr, test);
	ASSERT_STREQ("name", test->getName().c_str());
	ASSERT_STREQ("category", test->getCategory().c_str());
	ASSERT_STREQ("type", test->getType().c_str());
	if (test != nullptr)
	{
		delete test;
	}
}

TEST(FontBase, CopyAssignment)
{
	TestFontBase source("name", "category", "type");
	TestFontBase test("a", "b", "c");
	ASSERT_NO_THROW(test = source);
	ASSERT_STREQ(source.getName().c_str(), test.getName().c_str());
	ASSERT_STREQ(source.getCategory().c_str(), test.getCategory().c_str());
	ASSERT_STREQ(source.getType().c_str(), test.getType().c_str());
}