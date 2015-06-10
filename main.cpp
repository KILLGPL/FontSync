/// WIN32_LEAN_AND_MEAN prevents loading non-essential parts of the SDK
/// This is required to prevent out-of-order initialization of asio
#define WIN32_LEAN_AND_MEAN

#include <thread> /// standard C++ threading utilities
#include <chrono> /// standard C++ date/time utilities
#include <memory> /// standard C++ automatic memory management utilities

#include "Config.hpp"     /// fontsync service configuration
#include "FontSyncService.hpp"   /// fontsync service

// Internal name of the service
#define SERVICE_NAME             L"FontSync" 
 
// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"Font Synchronization Service" 
 
// Service start options.
#define SERVICE_START_TYPE       SERVICE_AUTO_START 
 
// List of service dependencies - "dep1\0dep2\0\0" 
#define SERVICE_DEPENDENCIES     L"" 
 
// The name of the account under which the service should run
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService" 
 
// The password to the service account name
#define SERVICE_PASSWORD         NULL 

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string errorString(DWORD errorMessageID)
{
    //Get the error message, if any.
    if(errorMessageID == 0)
        return "No error message has been recorded";

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

/**
 * Installs this application as a windows service
 */
DWORD installService()
{
    DWORD error = NO_ERROR;
    wchar_t szPath[MAX_PATH];
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    if(GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath)) != 0)
    {
        // Open the local default service control manager database 
        schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |  
            SC_MANAGER_CREATE_SERVICE); 
        if(schSCManager != NULL)
        {
            // Install the service into SCM by calling CreateService 
            schService = CreateServiceW( 
                schSCManager,                   // SCManager database 
                SERVICE_NAME,                   // Name of service 
                SERVICE_DISPLAY_NAME,           // Name to display 
                SERVICE_QUERY_STATUS,           // Desired access 
                SERVICE_WIN32_OWN_PROCESS,      // Service type 
                SERVICE_START_TYPE,             // Service start type 
                SERVICE_ERROR_NORMAL,           // Error control type 
                szPath,                         // Service binary 
                NULL,                           // No load ordering group 
                NULL,                           // No tag identifier 
                SERVICE_DEPENDENCIES,           // Dependencies 
                SERVICE_ACCOUNT,                // Service running account 
                SERVICE_PASSWORD                // Password of the account 
            );            
            if(schService == NULL)
            {
                error = GetLastError();
            }
        }
        else
        {
            error = GetLastError();       
        }
    }
    else
    {
        error = GetLastError();      
    }
    if(schSCManager != NULL) 
    { 
        CloseServiceHandle(schSCManager); 
    }
    if(schService != NULL)
    { 
        CloseServiceHandle(schService);
    }
    return error;
}

DWORD uninstallService()
{
    DWORD error = NO_ERROR;
    SC_HANDLE schSCManager = NULL; 
    SC_HANDLE schService = NULL; 
    SERVICE_STATUS ssSvcStatus = {}; 
 
    // Open the local default service control manager database 
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT); 
    if(schSCManager != NULL)
    {
        // Open the service with delete, stop, and query status permissions 
        schService = OpenServiceW(schSCManager, SERVICE_NAME, SERVICE_STOP |  
        SERVICE_QUERY_STATUS | DELETE);
        if(schService != NULL)
        {
            // Try to stop the service 
            if(ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
            { 
                Sleep(1000); 

                while (QueryServiceStatus(schService, &ssSvcStatus)) 
                { 
                    if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING) 
                    { 
                        Sleep(1000); 
                    } 
                    else break; 
                } 
            } 
            // Now remove the service by calling DeleteService. 
            if (!DeleteService(schService)) 
            {
                error = GetLastError();
            }
        }
        else
        {
            error = GetLastError();
        }
    }
    else
    {
        error = GetLastError();
    }
    if(schSCManager != NULL)
    {
        CloseServiceHandle(schSCManager);
    }
    if(schService != NULL)
    {
        CloseServiceHandle(schService);
    }
    return error;
}

int main(int argc, char** argv)
{
    if(argc > 1)
    {
        std::string operation = argv[1];
        if(operation == "install")
        {
            DWORD result = installService();
            if(result != NO_ERROR)
            {
                wprintf(L"installService failed w/err 0x%08lx\n", result);
                printf(errorString(result).c_str());
            }
            else
            {
                wprintf(L"successfully installed  %ls\n", SERVICE_DISPLAY_NAME);
            }
            return 0;
        }
        else if(operation == "uninstall")
        {
            DWORD result = uninstallService();
            if(result != NO_ERROR)
            {
                wprintf(L"uninstallService failed w/err 0x%08lx\n", result);
                printf(errorString(result).c_str());
            }
            else
            {
                wprintf(L"%ls successfully uninstalled\n", SERVICE_NAME);
            }
            return 0;
        }
    }
    wprintf(L"Parameters:\n"); 
    wprintf(L" install  to install the service.\n"); 
    wprintf(L" remove   to remove the service.\n"); 
    return 1;
}

