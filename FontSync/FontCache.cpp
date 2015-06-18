#include <iostream>

#include "FontCache.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#include <Windows.h>
#include <wingdi.h>

#include "Utilities.hpp"
#include "ServiceBase.hpp"

namespace detail
{
	struct IterableFontDirectory
	{
		boost::filesystem::path path;

		boost::filesystem::directory_iterator begin()
		{
			return boost::filesystem::directory_iterator(this->path);
		}

		boost::filesystem::directory_iterator end()
		{
			return boost::filesystem::directory_iterator();
		}

		IterableFontDirectory(const std::string& fontDirectory) : path(fontDirectory)
		{

		}
	};
}

struct FontCache::FontCacheImpl
{
	std::string fontDirectory;
	std::vector<LocalFont> cache;
	bool initialized;

	/**
	 * Cycles through all currently active fonts, unlinking them from the OS and removing 
	 * them from the in-memory cache.
	 * 
	 * @throws std::runtime_error if any fonts cannot be unloaded
	 *
	 * @note the exception behavior is subject to change depending on what we find out in the wild.
	 *
	 */
	void unloadCache()
	{
		unsigned int error = 0;
		for (auto file : cache)
		{
			if (RemoveFontResourceA(file.getLocalFile().c_str()) != 0)
			{
				/// Apparently multiple sessions using a font can force us to call this multiple times?
				/// Really is no clear documentation regarding it.  Thanks Microsoft.
				while (RemoveFontResourceA(file.getLocalFile().c_str()) != 0);
			}
			else
			{
				++error;
			}
		}
		/// Alert the top level OS windows that fonts have been updated
        PostMessage(HWND_BROADCAST, WM_FONTCHANGE, NULL, NULL);
		cache.clear();
		if(error > 0)
		{
			throw std::runtime_error(std::to_string(error) + " fonts failed to unload");
		}
	}

	/**
	 * Cycles through the local fonts directory, linking them with the OS and adding them 
	 * to the in-memory cache.
	 *
	 * @throws std::runtime_error if any fonts cannot be loaded
	 *
	 * @note the exception behavior is subject to change depending on what we find out in the wild.
	 *
	 */
	void updateCache()
	{
		unloadCache();
		unsigned int error = 0;
		for (auto file : detail::IterableFontDirectory(this->fontDirectory))
		{
			if (AddFontResourceA(file.path().string().c_str()) > 0)
			{
				/// TODO: learn how to extract font name/type data from winapi
				cache.push_back(LocalFont(file.path().string(), "N/A", "N/A", file.path().string()));
			}
			else
			{
				++error;
			}
		}
		/// Alert the top level OS windows that fonts have been updated
		PostMessage(HWND_BROADCAST, WM_FONTCHANGE, NULL, NULL);
		if(error > 0)
		{
			throw std::runtime_error(std::to_string(error) + " fonts failed to load");
		}
		initialized = true;
	}

	void synchronize(const std::vector<RemoteFont>& remoteFonts)
	{
		this->unloadCache();

		/// First look for any fonts that should be deleted.
		for (auto file : detail::IterableFontDirectory(this->fontDirectory))
		{
			bool remove = true;
			for (const auto& remote : remoteFonts)
			{
				if (file.path().filename() == remote.getName())
				{
					remove = false;
					break;
				}
			}
			if (remove)
			{
				boost::filesystem::remove(file);
			}
		}

		/// Next, check for updates and new fonts.
		for (const auto& font : remoteFonts)
		{
			std::stringstream ss;
            ss << this->fontDirectory << boost::filesystem::path::preferred_separator << font.getRemoteFile().substr(font.getRemoteFile().find_last_of("/\\") + 1);
			boost::filesystem::path localPath(ss.str());
			if (!boost::filesystem::exists(localPath) || md5(ss.str()) != font.getMD5())
			{
                std::wstringstream wss;
                CServiceBase::TEMP_INST->WriteEventLogEntry(wss.str().c_str(), EVENTLOG_INFORMATION_TYPE);
				download(ss.str(), font.getRemoteFile());
                wss << "Saved: " << localPath.c_str() << " to " << font.getRemoteFile().c_str();
            }
		}
		this->updateCache();
	}

	FontCacheImpl(const std::string& fontDirectory, bool cacheImmediately) : fontDirectory(fontDirectory), initialized(false)
	{
		if (!boost::filesystem::exists(boost::filesystem::path(fontDirectory)))
		{
			throw std::runtime_error("local font cache directory does not exist");
		}
		if (cacheImmediately)
		{
			updateCache();
		}
	}

	~FontCacheImpl()
	{
		unloadCache();
	}

};

FontCache::FontCache(const std::string& fontDirectory, bool cacheImmediately) : impl(new FontCacheImpl(fontDirectory, cacheImmediately))
{

}

bool FontCache::isInitialized() const
{
	return this->impl->initialized;
}

const std::vector<LocalFont>& FontCache::getCachedFonts() const
{
	return this->impl->cache;
}

void FontCache::synchronize(const std::vector<RemoteFont>& remoteFonts)
{
	this->impl->synchronize(remoteFonts);
}

void FontCache::updateCache()
{
	impl->updateCache();
}

FontCache::~FontCache()
{

}