#include "Config.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <boost/locale/encoding_utf.hpp>
#include <string>

struct Config::ConfigImpl
{
    std::string host;
    uint16_t port;
    uint32_t syncInterval;
	std::string resource;
    std::string localFontDirectory;

    ConfigImpl(const std::wstring& configFile)
    {
		try
		{
			boost::property_tree::ptree tree;
			boost::property_tree::ini_parser::read_ini(boost::locale::conv::utf_to_utf<char>(configFile.c_str(), configFile.c_str() + configFile.size()), tree);
			this->host = tree.get<std::string>("sync_server");
			this->port = tree.get<uint16_t>("port");
			this->syncInterval = tree.get<uint32_t>("sync_interval");
			this->resource = tree.get<std::string>("resource");
			this->localFontDirectory = tree.get<std::string>("local_font_dir");
		}
		catch (const boost::property_tree::ptree_error& error)
		{
			throw std::runtime_error(error.what());
		}
    }

	ConfigImpl(const std::string& host,
               uint16_t port, 
               uint32_t syncInterval, 
			   const std::string& resource,
               const std::string& localFontDirectory) :
			   host(host),
        port(port),
        syncInterval(syncInterval),
		resource(resource),
        localFontDirectory(localFontDirectory)
    {
        
    }
};

const std::string& Config::getHost() const
{
	return this->impl->host;
}
    
uint16_t Config::getPort() const
{
    return this->impl->port;
}

uint32_t Config::getSyncMillis() const
{
    return this->impl->syncInterval;
}

const std::string& Config::getResource() const
{
	return this->impl->resource;
}

const std::string& Config::getLocalFontDirectory() const
{
	return this->impl->localFontDirectory;
}

Config::Config() : 
    impl(new ConfigImpl("127.0.0.1", 80, 60000, "update.php", "C:\\FontSync\\Fonts"))
{

}

Config::Config(const std::wstring& configFile) : 
    impl(configFile.length() > 0 ? new ConfigImpl(configFile) : new ConfigImpl("127.0.0.1", 80, 60000, "update.php", "C:\\FontSync\\Fonts"))
{

}

Config::Config(const Config& other) : impl(new ConfigImpl(other.impl->host, other.impl->port, other.impl->syncInterval, other.impl->resource, other.impl->localFontDirectory))
{

}

Config& Config::operator=(Config& other)
{
	if (this != &other)
	{
		this->impl->host = other.impl->host;
		this->impl->port = other.impl->port;
		this->impl->syncInterval = other.impl->syncInterval;
		this->impl->resource = other.impl->resource;
		this->impl->localFontDirectory = other.impl->localFontDirectory;
	}
	return *this;
}

Config::~Config()
{

}