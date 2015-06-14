#include "Config.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <boost/locale/encoding_utf.hpp>
#include <string>

struct Config::ConfigImpl
{
    std::string syncServer;
    uint16_t port;
    uint32_t syncInterval;
    std::string localFontDirectory;

    ConfigImpl(const std::wstring& configFile)
    {
		try
		{
			boost::property_tree::ptree tree;
			boost::property_tree::ini_parser::read_ini(boost::locale::conv::utf_to_utf<char>(configFile.c_str(), configFile.c_str() + configFile.size()), tree);
			this->syncServer = tree.get<std::string>("sync_server");
			this->port = tree.get<uint16_t>("port");
			this->syncInterval = tree.get<uint32_t>("sync_interval");
			this->localFontDirectory = tree.get<std::string>("local_font_dir");
		}
		catch (const boost::property_tree::ptree_error& error)
		{
			throw std::runtime_error(error.what());
		}
    }

    ConfigImpl(const std::string& syncServer, 
               uint16_t port, 
               uint32_t syncInterval, 
               const std::string& localFontDirectory) :
        syncServer(syncServer),
        port(port),
        syncInterval(syncInterval),
        localFontDirectory(localFontDirectory)
    {
        
    }
};

const std::string& Config::getHost() const
{
    return this->impl->syncServer;
}
    
uint16_t Config::getPort() const
{
    return this->impl->port;
}

uint32_t Config::getSyncMillis() const
{
    return this->impl->syncInterval;
}

Config::Config() : 
    impl(new ConfigImpl("127.0.0.1", 80, 60000, ""))
{

}

Config::Config(const std::wstring& configFile) : 
    impl(configFile.length() > 0 ? new ConfigImpl(configFile) : new ConfigImpl("127.0.0.1", 80, 60000, ""))
{

}

Config::~Config()
{

}