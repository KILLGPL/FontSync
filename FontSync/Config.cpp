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
    unsigned int failedSyncDelay;
    unsigned int failedDownloadDelay;
    unsigned int failedDownloadRetries;

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
            this->failedSyncDelay = tree.get<unsigned int>("failed_sync_delay");
            this->failedDownloadDelay = tree.get<unsigned int>("failed_download_delay");
            this->failedDownloadRetries = tree.get<unsigned int>("failed_download_retries");
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
               const std::string& localFontDirectory,
               unsigned int failedSyncDelay,
               unsigned int failedDownloadDelay,
               unsigned int failedDownloadRetries) :
			   host(host),
        port(port),
        syncInterval(syncInterval),
		resource(resource),
        localFontDirectory(localFontDirectory),
        failedSyncDelay(failedSyncDelay),
        failedDownloadDelay(failedDownloadDelay),
        failedDownloadRetries(failedDownloadRetries)
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

unsigned int Config::getFailedSyncRetryDelay() const
{
    return this->impl->failedSyncDelay;
}

unsigned int Config::getFailedDownloadRetryDelay() const
{
    return this->impl->failedDownloadDelay;
}

unsigned int Config::getFailedDownloadRetryAttempts() const
{
    return this->impl->failedDownloadRetries;
}

Config::Config() : 
impl(new ConfigImpl("lukeleber.github.io", 80, 60000, "update.json", "C:\\windows\\Fonts", 60000, 3000, 3))
{

}

Config::Config(const std::wstring& configFile) : 
impl(configFile.length() > 0 ? new ConfigImpl(configFile) : new ConfigImpl("lukeleber.github.io", 80, 60000, "update.json", "C:\\windows\\Fonts", 60000, 3000, 3))
{

}

Config::Config(const Config& other) : impl(new ConfigImpl(other.impl->host, other.impl->port, other.impl->syncInterval, other.impl->resource, other.impl->localFontDirectory, other.impl->failedSyncDelay, other.impl->failedDownloadDelay, other.impl->failedDownloadRetries))
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
        this->impl->failedSyncDelay = other.impl->failedSyncDelay;
        this->impl->failedDownloadDelay = other.impl->failedDownloadDelay;
        this->impl->failedDownloadRetries = other.impl->failedDownloadRetries;
	}
	return *this;
}

Config::~Config()
{

}