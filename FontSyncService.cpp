#define WIN32_LEAN_AND_MEAN

#include "Config.hpp"
#include "FontSyncService.hpp"
#include "SyncClient.hpp"
#include <thread>


struct FontSyncService::FontSyncServiceImpl
{
    Config config;
    asio::io_service io_svc;
    volatile bool killswitch;
    HANDLE stoppedEvent;
    
    FontSyncServiceImpl(const Config& config) : config(config), killswitch(false)
    {
        stoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
        if(stoppedEvent == NULL) 
        { 
            // TODO: EEK!!!
        }
    }
    
    ~FontSyncServiceImpl()
    {
    if (stoppedEvent != NULL)
    {
        CloseHandle(stoppedEvent);
    }
    }
};

FontSyncService::FontSyncService(PWSTR pszServiceName,  
                                 BOOL fCanStop,  
                                 BOOL fCanShutdown,  
                                 BOOL fCanPauseContinue) : 
    CServiceBase(pszServiceName, 
                 fCanStop, 
                 fCanShutdown, 
                 fCanPauseContinue), 
    impl(new FontSyncServiceImpl(Config()))
{

}

void FontSyncService::OnStart(DWORD dwArgc, PWSTR *pszArgv)
{
    // Log a service start message to the Application log. 
    WriteEventLogEntry(L"FontSync is startint", EVENTLOG_INFORMATION_TYPE); 
    std::thread t([this](void)->void
    {
//            SyncClient client(config.getSyncServer(), config.getPort());
        while(!this->impl->killswitch)
        {
            std::cout << "tick" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(this->impl->config.getSyncInterval()));
        }
        // Signal the stopped event. 
        SetEvent(this->impl->stoppedEvent); 
    });
}
void FontSyncService::OnStop()
{
    // Log a service stop message to the Application log. 
    WriteEventLogEntry(L"FontSync is stopping", EVENTLOG_INFORMATION_TYPE); 

    // Indicate that the service is stopping and wait for the finish of the  
    // main service function (ServiceWorkerThread). 
    this->impl->killswitch = true;
    if (WaitForSingleObject(this->impl->stoppedEvent, INFINITE) != WAIT_OBJECT_0) 
    { 
        //TODO: EEK!
//            throw GetLastError(); 
    }
}

FontSyncService::~FontSyncService()
{

}
