#ifndef CONFIG_HPP
#define	CONFIG_HPP

#include <cstdint>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

class Config {

    std::string syncServer;
    uint16_t port;
    uint32_t syncInterval;
    std::string localFontDirectory;
    
public:
    
    const std::string& getSyncServer() const
    {
        return this->syncServer;
    }
    
    uint16_t getPort() const
    {
        return this->port;
    }
    
    uint32_t getSyncInterval() const
    {
        return this->syncInterval;
    }
    
    Config(const std::string& configFile)
    {
        boost::property_tree::ptree tree;
        boost::property_tree::ini_parser::read_ini(configFile, tree);
        this->syncServer = tree.get<std::string>("sync_server");
        this->port = tree.get<uint16_t>("port");
        this->syncInterval = tree.get<uint32_t>("sync_interval");
        this->localFontDirectory = tree.get<std::string>("local_font_dir");
    }
};

#endif	/* CONFIG_HPP */
