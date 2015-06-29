#include <atomic>
#include <chrono>
#include <csignal>
#include <string>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Config.hpp"
#include "FontCache.hpp"
#include "Logging.hpp"
#include "UpdateReceiver.hpp"

std::atomic_bool stop { false };

/// registers a function that is invoked when shutdown is signalled
/// this allows us to unlink from GDI for a 'clean' shutdown.
/// should make a proper shutdown sequence at some point, eh?
void registerSignals()
{
    auto handler = [](int sig)->void
    {
        stop = true;
        FONTSYNC_LOG_TRIVIAL(info) << "Shutting down...forcing kill in 5 seconds.";
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    };
    signal(SIGINT,   handler);
    signal(SIGBREAK, handler);
    signal(SIGTERM,  handler);
    signal(SIGABRT,  handler);
    signal(SIGTERM,  handler);
}

/**
 * Entry point for the executable.
 * 
 * @param argc the number of arguments provided by the host environment
 * 
 * @param argv the arguments provided by the host environment
 * 
 * @return 0 upon success, non-zero upon failure
 *
 * @note a configuration file path can be provided as an optional argument
 *       for more information, refer to the enclosed config documentation.
 * 
 */
int main(int argc, char** argv)
{
    ShowWindow(GetConsoleWindow(), SW_SHOW);
    try
    {
        Config config(argc > 1 ? argv[1] : "");
        initLogging(config);
        FontCache fontCache(config.get<std::string>("local_font_dir"), 
                                 config.get<int>("failed_download_delay"), 
                                 config.get<int>("failed_download_retries"));
        UpdateReceiver receiver(config.get<std::string>("host"), 
                                config.get<int>("port"), 
                                config.get<std::string>("resource"));
        registerSignals();
        auto lastSync = std::chrono::system_clock::now() - std::chrono::milliseconds(config.get<int>("sync_interval"));
        do
        {
            auto now = std::chrono::system_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSync).count() > config.get<int>("sync_interval"))
            {
                lastSync = std::chrono::system_clock::now();
                try
                {
                    fontCache.synchronize(receiver.getRemoteFontIndex());
                    FONTSYNC_LOG_TRIVIAL(info) << "Font Synchronization Complete";
                }
                catch (const std::runtime_error& e)
                {
                    FONTSYNC_LOG_TRIVIAL(error) << "Font Synchronization Failed: " << e.what();
                    lastSync = std::chrono::system_clock::now() - std::chrono::milliseconds(config.get<int>("sync_interval") - config.get<int>("failed_sync_delay"));
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            }
        } while (!stop);
    }
    catch (const std::runtime_error& error)
    {
        FONTSYNC_LOG_TRIVIAL(fatal) << "Fatal Exception: " << error.what();
    }
    catch (...)
    {
        FONTSYNC_LOG_TRIVIAL(fatal) << "Unknown Fatal Exception";
    }
    return 0;
}
