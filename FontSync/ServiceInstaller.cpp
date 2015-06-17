#include "ServiceInstaller.hpp"

struct ServiceInstaller::ServiceInstallerImpl
{
    LPWSTR name;
	LPWSTR displayName;
	WORD startType;
	LPWSTR dependencies;
	LPWSTR account;
	LPWSTR password;

	ServiceInstallerImpl(LPWSTR name,
						 LPWSTR displayName,
						 WORD startType,
						 LPWSTR dependencies,
						 LPWSTR account,
						 LPWSTR password) :
		name(name),
		displayName(displayName),
		startType(startType),
		dependencies(dependencies),
		account(account),
		password(password)
	{

	}
};

ServiceInstaller::ServiceInstaller(LPWSTR name, 
	                               LPWSTR displayName, 
								   WORD startType, 
								   LPWSTR dependencies, 
								   LPWSTR account, 
								   LPWSTR password) :
	impl(new ServiceInstallerImpl(name,
								  displayName,
								  startType,
								  dependencies,
								  account,
								  password))
{

}

DWORD ServiceInstaller::install()
{
	DWORD error = NO_ERROR;
	wchar_t szPath[MAX_PATH];
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	if (GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath)) != 0)
	{
		// Open the local default service control manager database 
		schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
			SC_MANAGER_CREATE_SERVICE);
		if (schSCManager != NULL)
		{
			// Install the service into SCM by calling CreateService 
			schService = CreateServiceW(
				schSCManager,                     // SCManager database 
				this->impl->name,         // Name of service 
				this->impl->displayName,  // Name to display 
				SERVICE_QUERY_STATUS,             // Desired access 
				SERVICE_WIN32_OWN_PROCESS,        // Service type 
				this->impl->startType,            // Service start type 
				SERVICE_ERROR_NORMAL,             // Error control type 
				szPath,                           // Service binary 
				NULL,                             // No load ordering group 
				NULL,                             // No tag identifier 
				this->impl->dependencies, // Dependencies 
				this->impl->account,      // Service running account 
				this->impl->password      // Password of the account 
				);
			if (schService == NULL)
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
	if (schSCManager != NULL)
	{
		CloseServiceHandle(schSCManager);
	}
	if (schService != NULL)
	{
		CloseServiceHandle(schService);
	}
	return error;
}

DWORD ServiceInstaller::uninstall()
{
	DWORD error = NO_ERROR;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {};

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager != NULL)
	{
		schService = OpenServiceW(schSCManager, this->impl->name, SERVICE_STOP |
			SERVICE_QUERY_STATUS | DELETE);
		if (schService != NULL)
		{
			if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
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
	if (schSCManager != NULL)
	{
		CloseServiceHandle(schSCManager);
	}
	if (schService != NULL)
	{
		CloseServiceHandle(schService);
	}
	return error;
}

ServiceInstaller::~ServiceInstaller()
{

}