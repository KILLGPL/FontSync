#ifndef FONT_SYNC_SERVICE_HPP_INCLUDED
#define	FONT_SYNC_SERVICE_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <memory>
#include "ServiceBase.hpp"

/**
 * A service adapter class that synchronizes a font cache across a network
 * 
 */
class FontSyncService : public CServiceBase 
{
	/// Private Implementation
    struct FontSyncServiceImpl;
    
    /// Private Implementation
    std::unique_ptr<FontSyncServiceImpl> impl;

 protected:
     
    /**
     * Invoked when Windows commands this service to start
     * 
     * @param dwArgc the number of arguments
     * 
     * @param pszArgv the arguments
     * 
     */
    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
    
    /**
     * Invoked when Windows commands this service to stop
     * 
     */
    virtual void OnStop();
    
public:

    static FontSyncService* TEMP_INST;
    /**
     * Constructs a FontSyncService (Invoked by Windows)
     * 
     * @param pszServiceName the internal name of this service
     * 
     */
    FontSyncService(PWSTR pszServiceName);
        
    /**
     * Virtual Destructor
     * 
     */
    virtual ~FontSyncService();
};

#endif
