#ifndef SERVICE_INSTALLER_HPP_INCLUDED
#define SERVICE_INSTALLER_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <memory>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/**
 * Helper class that eases the installation / uninstallation of windows services.
 *
 */
class ServiceInstaller
{
	/// Private Implementation
	struct ServiceInstallerImpl;

	/// Private Implementation
	std::unique_ptr<ServiceInstallerImpl> impl;

public:

	/**
	 * Constructs a ServiceInstaller with the provided service name, 
	 * display name, start type, dependencies, account, and password.
	 *
	 * @param name the name of the service
	 *
	 * @param displayName the display name of the service
	 *
	 * @param startType the service startup type of the service
	 *
	 * @param dependencies the dependencies of the service
	 *
	 * @param account the user account to run the service under
	 *
	 * @param password the password of the user account
	 *
	 */
    ServiceInstaller(const LPWSTR name,
                     const LPWSTR displayName,
					 WORD startType = SERVICE_AUTO_START, 
                     const LPWSTR dependencies = NULL,
					 const LPWSTR account = NULL, 
                     const LPWSTR password = NULL);

	/**
	* Attempts to install this application as a windows service
	*
	* @return 0 upon success, non-zero upon failure
	*
	*/
	DWORD install();

	/**
	* Attempts to uninstall this application from the local SCM
	*
	* @note this method may block
	*
	* @return 0 upon success, non-zero upon failure
	*
	*/
	DWORD uninstall();

	/**
	 * Default Destructor
	 *
	 */
	~ServiceInstaller();
};

#endif
