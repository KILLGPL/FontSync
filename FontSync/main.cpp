#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Internal name of the service
#define SERVICE_NAME             L"FontSync" 
 
// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"Font Synchronization Service" 
 
#include "ServiceInstaller.hpp"
#include "FontSyncService.hpp"
#include "Utilities.hpp"

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
            DWORD result = ServiceInstaller(SERVICE_NAME, SERVICE_DISPLAY_NAME).install();
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
            DWORD result = ServiceInstaller(SERVICE_NAME, SERVICE_DISPLAY_NAME).uninstall();
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

