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
	FontCache(const std::string& fontDirectory, bool cacheImmediately = false);

	/**
	 * Is this cache initialized?
	 * 
	 * @return true if this cache is initialized, otherwise false
	 *
	 */
	bool isInitialized() const;

	/**
	 * Retrieves a read-only view of the contents of this cache
	 *
	 * @return a read-only view of the contents of this cache
	 *
	 */
	const std::vector<LocalFont>& getCachedFonts() const;

	/**
	 * Updates the contents of this cache.  More formally, the cache is 
	 * emptied (unlinking all fonts with the operating system), and repoulated 
	 * with the contents of its managed directory.
	 *
	 * @throws std::runtime_error if any re-caching error occurs
	 *
	 */
	void updateCache();

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
