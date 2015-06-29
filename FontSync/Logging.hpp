#ifndef LOGGING_HPP
#define LOGGING_HPP
#include <boost/log/trivial.hpp>
class Config;

bool fontsync_logging_initialized();

void initLogging(const Config& config);

#define FONTSYNC_LOG_TRIVIAL(X) \
    if(!fontsync_logging_initialized()) \
    { \
        BOOST_LOG_TRIVIAL(warning) << "Logging configuration is not complete."; \
    } \
    BOOST_LOG_TRIVIAL(X)

#endif