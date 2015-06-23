#ifndef NDEBUG
#include <iostream>
#endif
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

 
#include "Utilities.hpp"
#include "FontCache.hpp"
#include "UpdateReceiver.hpp"
#include <thread>
#include <chrono>
#include <sstream>

/// Stop Flag
volatile bool shouldStop = false;

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
 * 
 */
int main(int argc, char** argv)
{
    try
    {
        Config config(argc > 1 ? argv[1] : "");

        FontCache localFontCache(config.getLocalFontDirectory(), 
                                 config.getFailedDownloadRetryDelay(), 
                                 config.getFailedDownloadRetryAttempts(), 
                                 shouldStop);

        UpdateReceiver receiver(config.getHost(), 
                                config.getPort(), 
                                config.getResource());


        /// make sure we run right away the first time by setting the 'lastSync' far enough in the past.
        auto lastSync = std::chrono::system_clock::now() - std::chrono::milliseconds(config.getSyncMillis());
        do
        {
            auto now = std::chrono::system_clock::now();

            /// time to sync up?
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSync).count() > config.getSyncMillis())
            {
                lastSync = std::chrono::system_clock::now();

                /// errors thrown from here should not be fatal, so simply log 'em and soldier on.
                try
                {
                    /// lazily initialize local cache on the first run
                    if (!localFontCache.isInitialized())
                    {
                        localFontCache.updateCache();
                    }

                    /// try to synchronize
                    localFontCache.synchronize(receiver.getRemoteFontIndex());
                }
                catch (const std::runtime_error& error)
                {
#ifndef NDEBUG
                    std::cerr << "Font Synchronization Failed: " << error.what() << std::endl;
#endif
                    lastSync = std::chrono::system_clock::now() - std::chrono::milliseconds(config.getSyncMillis() - config.getFailedSyncRetryDelay());
                }
            }
            else
            {
                /// sleep for 1 second if it's not time for a sync yet.
                /// this allows us to respond to stop commands 
                /// every second or so rather than every sync interval
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        } while (!shouldStop);
    }
    catch (const std::runtime_error& error)
    {
        std::wstringstream wss;
        wss << L"Unexpected Exception(outer): " << error.what();
        WriteEventLogEntry(wss.str().c_str());
    }
    catch (...)
    {
#ifndef NDEBUG
        std::cerr << "unknown error" << std::endl;
#endif
    }

    return 0;
}

