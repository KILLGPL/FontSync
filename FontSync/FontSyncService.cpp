#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "Config.hpp"
#include "FontCache.hpp"
#include "FontSyncService.hpp"
#include "UpdateReceiver.hpp"
#include "Utilities.hpp"

/**
 * The private implementation of the FontSyncService class.
 * 
 */
struct FontSyncService::FontSyncServiceImpl
{
    
    /// a volatile flag signaling that this service should stop ASAP
    volatile bool shouldStop;

    /// a signal event to send back to Windows indicating a graceful stop
    HANDLE stoppedEvent;
    
    /**
     * Default Constructor
     * 
     * @throws std::runtime_error if Windows cannot create an event
     */
    FontSyncServiceImpl() : shouldStop(false)
    {
        stoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
        if(stoppedEvent == NULL) 
        { 
            throw std::runtime_error("unable to create stop signal event");
        }
    }
    
    /**
     * Destructor
     */
    ~FontSyncServiceImpl()
    {
        if (stoppedEvent != NULL)
        {
            CloseHandle(stoppedEvent);
        }
    }
};

/**
 * Constructs a FontSyncService (Invoked by Windows)
 * 
 * @param pszServiceName the internal name of this service
 * 
 */
FontSyncService::FontSyncService(LPWSTR pszServiceName) : 
    CServiceBase(pszServiceName, TRUE, TRUE, FALSE), 
    impl(new FontSyncServiceImpl())
{

}

/// workaround for...
///
/// Config config(((pszArgv == nullptr) ? L"" : pszArgv[1]));
///
/// crashing when pszArgv == nullptr
///
LPWSTR getConfigPath(DWORD dwArgc, PWSTR* pszArgv)
{
    if (pszArgv == nullptr)
    {
        return L"";
    }
    else
    {
        return pszArgv[1];
    }
}

/**
 * Invoked when Windows commands this service to start
 * 
 * @param dwArgc the number of arguments
 * 
 * @param pszArgv the arguments
 * 
 */
void FontSyncService::OnStart(DWORD dwArgc, PWSTR *pszArgv)
{
       	std::thread([&, this](void)->void{
        try
        {
            /// According to the win32 documentation, pszArgv can be null if no arguments are 
            /// passed, or will contain the service name at index 0 followed by the arguments 
            /// if any are provided...
            Config config(getConfigPath(dwArgc, pszArgv));
            
            FontCache localFontCache(config.getLocalFontDirectory());
            UpdateReceiver receiver(config.getHost(), config.getPort(), config.getResource());
            
            /// make sure we run the first time by setting the 'lastSync' 
            /// far enough in the past.
            auto lastSync = std::chrono::system_clock::now() -
                std::chrono::milliseconds(config.getSyncMillis());
            do
            {
                /// check if it's time to sync up
                auto now = std::chrono::system_clock::now();
                if (std::chrono::duration_cast<
                    std::chrono::milliseconds>(now - lastSync).count() > config.getSyncMillis())
                {
                    this->WriteEventLogEntry(L"ITERATION 2", EVENTLOG_INFORMATION_TYPE);
                    lastSync = std::chrono::system_clock::now();
                    try
                    {
                        this->WriteEventLogEntry(L"ITERATION 3", EVENTLOG_INFORMATION_TYPE);
                        if (!localFontCache.isInitialized())
                        {
                            this->WriteEventLogEntry(L"ITERATION 4", EVENTLOG_INFORMATION_TYPE);
                            localFontCache.updateCache();
                        }
                        this->WriteEventLogEntry(L"ITERATION 5", EVENTLOG_INFORMATION_TYPE);
                        localFontCache.synchronize(receiver.getRemoteFontIndex());
                        this->WriteEventLogEntry(L"ITERATION 6", EVENTLOG_INFORMATION_TYPE);
                    }
                    catch (const std::runtime_error& error)
                    {
                        std::wstringstream wss;
                        wss << L"Unexpected Exception: " << error.what();
                        this->WriteEventLogEntry(wss.str().c_str(), EVENTLOG_ERROR_TYPE);
                        /// Choke.  Give it 60 seconds and try again.
                        lastSync = std::chrono::system_clock::now() - std::chrono::milliseconds(config.getSyncMillis() - 60000);
                    }
                }
                else
                {
                    /// sleep for 1 second if it's not time for a sync yet.
                    /// this allows us to respond to Windows stop commands 
                    /// every second or so rather than every sync interval
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
            } while (!this->impl->shouldStop);

            /// fire off an event to notify Windows that shutdown is complete
        }
        catch (const std::runtime_error& error)
        {
            std::wstringstream wss;
            wss << L"Unexpected Exception(outer): " << error.what();
            this->WriteEventLogEntry(wss.str().c_str(), EVENTLOG_ERROR_TYPE);
        }
        catch (...)
        {
            this->WriteEventLogEntry(L"Caught some other kind of monster", EVENTLOG_ERROR_TYPE);
        }
	}).detach();
    SetEvent(this->impl->stoppedEvent);
}

/**
 * Invoked when Windows commands this service to stop
 * 
 */
void FontSyncService::OnStop()
{
    /// signal our boss thread to stop
    this->impl->shouldStop = true;
    
    /// wait (indefinitely) for the boss thread to stop
    if (WaitForSingleObject(this->impl->stoppedEvent, 
                            INFINITE) != WAIT_OBJECT_0) 
    {
        throw std::runtime_error("interrupted while shutting down");
    }
    
    /// this function returns to Windows -- so by now "mission complete"
}

/**
 * Virtual Destructor
 * 
 */
FontSyncService::~FontSyncService()
{

}
