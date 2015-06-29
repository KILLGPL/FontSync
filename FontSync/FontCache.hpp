#ifndef FONT_CACHE_HPP_INCLUDED
#define FONT_CACHE_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <memory>
#include <vector>
#include "LocalFont.hpp"
#include "RemoteFont.hpp"

/**
 * An in-memory cache that links its managed fonts to the operating system 
 * only for the lifetime of the service.  When the object is either refreshed, 
 * is deleted, or goes out of scope; all associated fonts are automatically 
 * unlinked from the operating system.
 *
 */
class FontCache
{
	/// Private Implementation
	struct FontCacheImpl;

	/// Private Implementation
	std::unique_ptr<FontCacheImpl> impl;

public:

	/**
	 * Constructs a FontCache that manages the provided directory
	 *
	 * @param fontDirectory the directory that this cache should manage
	 *
	 * @param cacheImmediately should the fonts be cached immediately?
	 *
	 * @throws std::runtime_error if any caching error occurs
	 *
	 */
	FontCache(const std::string& fontDirectory, unsigned int failedDownloadRetryDelay, unsigned int failedDownloadRetryAttempts);

	/**
	 * Synchronizes this cache with its remote counterpart.
	 *
	 * @throws std::runtime_error if any synchronization error occurs
	 *
	 */
	void synchronize(const std::vector<RemoteFont>& remoteFonts);
    
	/**
	 * Default Destructor
	 *
	 */
	~FontCache();
};

#endif
