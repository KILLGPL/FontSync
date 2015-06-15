#include <gtest/gtest.h>
#include "../FontSync/Config.cpp"

TEST(ConfigTest, DefaultConstructor)
{
	Config config;
	ASSERT_STREQ("127.0.0.1", config.getHost().c_str());
	ASSERT_EQ(80, config.getPort());
	ASSERT_EQ(60000, config.getSyncMillis());
}

TEST(ConfigTest, INIConstructor_Valid)
{
	Config* config = nullptr;
	try
	{
		ASSERT_NO_THROW(config = new Config(L"valid_config.ini"));
	}
	catch (...) { /*ignore*/ }
	if (config)
	{
		delete config;
	}
}

TEST(ConfigTest, INIConstructor_Invalid)
{
	Config* config = nullptr;
	try
	{
		ASSERT_THROW(config = new Config(L"invalid_config.ini"), std::runtime_error);
	}
	catch (...) { /*ignore*/ }
	if (config)
	{
		delete config;
	}
	try
	{
		ASSERT_THROW(config = new Config(L"invalid_config_file_not_found.ini"), std::runtime_error);
	}
	catch (...) { /*ignore*/ }
	if (config)
	{
		delete config;
	}
}
