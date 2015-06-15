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
		Config config(dwArgc > 0 ? pszArgv[1] : L"");
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
				lastSync = std::chrono::system_clock::now();
				try
				{
					if (!localFontCache.isInitialized())
					{
						localFontCache.updateCache();
					}
					std::vector<LocalFont> locals = localFontCache.getCachedFonts();
					std::vector<RemoteFont> remotes = receiver.getRemoteFontIndex();
					/// TODO: Hash out synchronizer logic and this project is done.
				}
				catch (const std::runtime_error& error)
				{
					std::wstringstream wss;
					wss << L"Unexpected Exception: " << error.what();
					this->WriteEventLogEntry(wss.str().c_str(), EVENTLOG_ERROR_TYPE);
					wss << "Please call helpdesk for assistance.";
					MessageBoxW(NULL, wss.str().c_str(), L"error", MB_ICONWARNING);
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
		SetEvent(this->impl->stoppedEvent);
	}).join();
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
