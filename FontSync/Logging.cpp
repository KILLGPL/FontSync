#include "Logging.hpp"
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sinks.hpp>

#include <boost/log/utility/setup.hpp>

#include "Config.hpp"

bool initialized = false;

bool fontsync_logging_initialized()
{
    return initialized;
}


void initLogging(const Config& config)
{
    if (config.get<bool>("console_logging_enabled"))
    {
        boost::log::add_console_log(std::cout,
            boost::log::keywords::format = config.get<std::string>("console_logging_format"),
            boost::log::keywords::auto_flush = true);
    }
    if (config.get<bool>("file_logging_enabled"))
    {
        boost::log::add_file_log(
            boost::log::keywords::file_name = config.get<std::string>("file_name_format"),
            boost::log::keywords::rotation_size = config.get<int>("max_individual_file_size"),
            boost::log::keywords::max_size = config.get<int>("max_cumulative_file_size"),
            boost::log::keywords::format = config.get<std::string>("file_logging_format"),
            boost::log::keywords::auto_flush = true);
    }
    boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= config.get<boost::log::trivial::severity_level>("logging_severity_filter"));
    initialized = true;
}