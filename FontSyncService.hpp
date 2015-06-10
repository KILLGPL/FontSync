/* 
 * File:   FontSync.hpp
 * Author: luke
 *
 * Created on June 9, 2015, 10:22 PM
 */

#ifndef FONTSYNC_HPP
#define	FONTSYNC_HPP

#include "asio.hpp"
#include "SyncClient.hpp"
#include "ServiceBase.hpp"

class FontSyncService : public CServiceBase 
{
    Config config;
    asio::io_service io_svc;
    volatile bool killswitch;
    HANDLE stoppedEvent;
protected:
    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv)
    {
        // Log a service start message to the Application log. 
        WriteEventLogEntry(L"CppWindowsService in OnStart",  
            EVENTLOG_INFORMATION_TYPE); 
        this->killswitch = false;
        std::thread t([this](void)->void
        {
//            SyncClient client(config.getSyncServer(), config.getPort());
            while(!killswitch)
            {
                std::cout << "tick" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(config.getSyncInterval()));
            }
            // Signal the stopped event. 
            SetEvent(stoppedEvent); 
        });
    }
    virtual void OnStop()
    {
        // Log a service stop message to the Application log. 
        WriteEventLogEntry(L"CppWindowsService in OnStop",  
            EVENTLOG_INFORMATION_TYPE); 

        // Indicate that the service is stopping and wait for the finish of the  
        // main service function (ServiceWorkerThread). 
        killswitch = TRUE; 
        if (WaitForSingleObject(stoppedEvent, INFINITE) != WAIT_OBJECT_0) 
        { 
            //TODO: EEK!
//            throw GetLastError(); 
        }
    }
public:
    
    FontSyncService(const Config& config, PWSTR pszServiceName,  
        BOOL fCanStop = TRUE,  
        BOOL fCanShutdown = TRUE,  
        BOOL fCanPauseContinue = FALSE) : CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue), config(config)
    {
        stoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
        if (stoppedEvent == NULL) 
        { 
            // TODO: EEK!!!
        }
    }
        
    virtual ~FontSyncService()
    {
        if (stoppedEvent != NULL)
        {
            CloseHandle(stoppedEvent);
        }
    }
};

#endif	/* FONTSYNC_HPP */

