#include "Config.hpp"

#include <map>
#include <string>

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Logging.hpp"

/// quick and dirty lookup table to convert strings to logging constants
std::map<std::string, boost::log::trivial::severity_level> severityMappings = 
{
    { "fatal",   boost::log::trivial::fatal }, 
    { "error",   boost::log::trivial::error }, 
    { "warning", boost::log::trivial::warning }, 
    { "info",    boost::log::trivial::info }, 
    { "debug",   boost::log::trivial::debug }, 
    { "trace",   boost::log::trivial::trace }, 
};



struct Config::ConfigImpl
{
    std::map<std::string, std::pair<boost::any, boost::any>> properties;

    bool onConfigError(const std::exception& e, int options = MB_CANCELTRYCONTINUE | MB_ICONERROR | MB_DEFBUTTON3 | MB_SYSTEMMODAL)
    {
        std::stringstream ss;
        ss << "An error has occured during fontsync configuration:\n" << e.what() << "\n\nI can try to continue with my default settings, retry loading again after you fix the issue, or just throw my hands up and quit.\n\nWhat do you want me to do?";
        FONTSYNC_LOG_TRIVIAL(error) << "An error has occured during fontsync configuration (" << e.what() << ")";
        switch (MessageBoxA(nullptr, ss.str().c_str(), "Configuration Error", options))
        {
        case IDTRYAGAIN:
            FONTSYNC_LOG_TRIVIAL(info) << "re-attempting to apply user configuration...";
            return true;
        case IDCONTINUE:
            FONTSYNC_LOG_TRIVIAL(info) << "falling back to default configuration...";
            return false;
        default:
            exit(1);
        }
    }

    void populate(boost::property_tree::ptree& tree) {/* no-op -- terminate meta-recursion */ }

    template<typename Key, typename Value, typename... Remaining>
    void populate(boost::property_tree::ptree& tree, Key&& key, Value&& value, Remaining&&... args)
    {
        this->properties.insert(std::make_pair(key, std::make_pair(tree.get(key, value), value)));
        populate(tree, args...);
    }

    ConfigImpl(const std::map<std::string, std::pair<boost::any, boost::any>> properties) : properties(properties)
    {

    }

    ConfigImpl(const std::string& configFile)
    {
        boost::property_tree::ptree tree;
        bool tryAgain;
        do
        {
            try
            {
                tryAgain = false;
                auto extension = boost::filesystem::path(configFile).extension();
                /// HACK!  due to variations between boost::property_tree::*_parser::read_*...
                ///        the various parsers can't be loaded into a string indexed map...
                if (extension == ".ini")
                {
                    boost::property_tree::ini_parser::read_ini(configFile, tree);
                }
                else if (extension == ".json")
                {
                    boost::property_tree::json_parser::read_json(configFile, tree);
                }
                else if (extension == ".xml")
                {
                    boost::property_tree::xml_parser::read_xml(configFile, tree);
                }
                else
                {
                    tryAgain = this->onConfigError(std::runtime_error(extension.string() + (" configuration files are not supported")), MB_OKCANCEL | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
                }
            }
            catch (const boost::property_tree::ptree_error& error)
            {
                tryAgain = this->onConfigError(error);
            }
        } while (tryAgain);

        /// Soldier on, even if configuration fails...
        /// we'll just try out the defaults
        this->populate(tree,
            "host", "localhost",
            "port", 80,
            "sync_interval", 60000,
            "resource", "update.php",
            "local_font_dir", "C:\\windows\\fonts",
            "failed_sync_delay", 60000,
            "failed_download_delay", 5000,
            "failed_download_retries", 3,
            "console_logging_enabled", true,
            "console_logging_format", "[%TimeStamp%]: %Message%",
            "file_logging_enabled", true,
            "file_name_format", "FontSync_%3N.log",
            "max_individual_file_size", 1 * 1024 * 1024,
            "max_cumulative_file_size", 20 * 1024 * 1024,
            "file_logging_format", "[%TimeStamp%]: %Message%",
            "logging_severity_filter", "info"
        );

        /// Special case for boost::log::trivial::severity_level
        try
        {
            auto iter = severityMappings.find(boost::any_cast<std::string>(this->properties["logging_severity_filter"].first));
            this->properties["logging_severity_filter"].first = iter != severityMappings.end() ? iter->second : boost::log::trivial::info;
            this->properties["logging_severity_filter"].second = iter != severityMappings.end() ? iter->second : boost::log::trivial::info;
        }
        catch (const boost::bad_any_cast&)
        {
            this->properties["logging_severity_filter"].first = boost::log::trivial::info;
            this->properties["logging_severity_filter"].second = boost::log::trivial::info;
        }
    }
};

#define EXPLICIT_INSTANTIATE(X) \
template<> \
X Config::get(const std::string& option) const { \
    try \
    { \
        return boost::any_cast<X>(this->impl->properties.at(option).first); \
    } \
    catch(...) \
    { \
        std::cout << "Mucked Up Config" << std::endl; \
        return boost::any_cast<X>(this->impl->properties.at(option).second); \
    } \
}

EXPLICIT_INSTANTIATE(boost::log::trivial::severity_level)
EXPLICIT_INSTANTIATE(std::string)
EXPLICIT_INSTANTIATE(unsigned)
EXPLICIT_INSTANTIATE(signed)
EXPLICIT_INSTANTIATE(bool)
EXPLICIT_INSTANTIATE(const char*)

Config::Config(const std::string& configFile) : 
impl(new ConfigImpl(configFile))
{

}

Config::Config(const Config& other) : impl(new ConfigImpl(other.impl->properties))
{

}

Config& Config::operator=(Config& other)
{
	if (this != &other)
	{
        this->impl->properties = other.impl->properties;
	}
	return *this;
}

Config::~Config()
{

}