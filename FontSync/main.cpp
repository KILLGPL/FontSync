#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

/**
 * Retrieves the error associated with the provided error code
 * 
 * @param errorCode the error code to look up
 * 
 * @return the error message of the provided error code
 * 
 */
std::wstring errorString(DWORD errorCode)
{
    if(errorCode == 0)
    {
        return L"no error";        
    }
    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                 FORMAT_MESSAGE_FROM_SYSTEM | 
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, 
                                 errorCode, 
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                                 (LPWSTR)&messageBuffer, 0, NULL);
    std::wstring message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

/**
 * Attempts to install this application as a windows service
 * 
 * @return 0 upon success, non-zero upon failure
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

/**
 * Attempts to uninstall this application from the local SCM
 * 
 * @note this method may block
 * 
 * @return 0 upon success, non-zero upon failure
 */
DWORD uninstallService()
{
    DWORD error = NO_ERROR;
    SC_HANDLE schSCManager = NULL; 
    SC_HANDLE schService = NULL; 
    SERVICE_STATUS ssSvcStatus = {}; 
 
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT); 
    if(schSCManager != NULL)
    {
        schService = OpenServiceW(schSCManager, SERVICE_NAME, SERVICE_STOP |  
        SERVICE_QUERY_STATUS | DELETE);
        if(schService != NULL)
        {
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

#include "FontSyncService.hpp"

/**
 * Entry point for the executable.
 * 
 * To install the service, pass in the string "install".
 * To uninstall the service, pass in the string "uninstall"
 * 
 * Windows will start the service for you via the installed SCM.
 * Don't try this at home, kids.
 * 
 * @param argc the number of arguments provided by the host environment
 * 
 * @param argv the arguments provided by the host environment
 * 
 * @return 0 upon success, non-zero upon failure
 * 
 */
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
                wprintf(errorString(result).c_str());
            }
            else
            {
                wprintf(L"%ls installed\n", SERVICE_DISPLAY_NAME);
            }
            return 0;
        }
        else if(operation == "uninstall")
        {
            DWORD result = uninstallService();
            if(result != NO_ERROR)
            {
                wprintf(L"uninstallService failed w/err 0x%08lx\n", result);
                wprintf(errorString(result).c_str());
            }
            else
            {
                wprintf(L"%ls uninstalled\n", SERVICE_NAME);
            }
            return 0;
        }
        else
        {
            wprintf(L"Parameters:\n"); 
            wprintf(L" install  to install the service.\n"); 
            wprintf(L" remove   to remove the service.\n"); 
            return 1;            
        }
    }

    FontSyncService service(SERVICE_NAME); 
    if (!CServiceBase::Run(service)) 
    { 
        wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError()); 
    }
    return 0;
}

