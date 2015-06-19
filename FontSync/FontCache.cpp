#include "FontCache.hpp"

#include <sstream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#include <Windows.h>
#include <wingdi.h>

#include "Utilities.hpp"
#include "ServiceBase.hpp"
#include <thread>

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
    /// The service to report errors to
    const CServiceBase* service;

    /// The directory to use for fonts
	std::string fontDirectory;

    /// The internal cache containing the fonts loaded by GDI
	std::vector<LocalFont> cache;

    /// The delay between retrying a failed download
    unsigned int failedDownloadRetryDelay;

    /// The number of times to try a download before aborting
    unsigned int failedDownloadRetryAttempts;

    /// Reference to the service killswitch
    volatile bool& shouldStop;

    /// Initialized yet?
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
				/// apparently multiple sessions using a font can force us to call this multiple times?
                /// keep a close eye on this...smells like an infinite loop to me if things break.
				while (RemoveFontResourceA(file.getLocalFile().c_str()) != 0);
			}
			else
			{
				++error;
			}
		}
        {
            DWORD hack = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
            /// Alert the top level OS windows that fonts have been updated
            BroadcastSystemMessage(BSF_IGNORECURRENTTASK | BSF_POSTMESSAGE, &hack, WM_FONTCHANGE, NULL, NULL);
        }
		cache.clear();

        /// should be viewed as a 'catastrophic' error.  if we can't unlink our fonts with GDI then 
        /// we surely shouldn't be trying to overwrite them...perhaps in the future we can look into 
        /// whether or not this is truly an unrecoverable situation, but for now just abort the sync.
		if(error > 0)
		{
            //throw std::runtime_error(std::to_string(error) + " fonts failed to unload");
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
        {
            DWORD hack = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
            /// Alert the top level OS windows that fonts have been updated
            BroadcastSystemMessage(BSF_IGNORECURRENTTASK | BSF_POSTMESSAGE, &hack, WM_FONTCHANGE, NULL, NULL);
        }


        /// should be viewed as a 'catastrophic' error.  if we can't link our fonts with GDI then 
        /// one or more of the fonts must have been corrupted...perhaps in the future we can look into 
        /// whether or not this is truly an unrecoverable situation, but for now just abort the sync.
        if(error > 0)
		{
			//throw std::runtime_error(std::to_string(error) + " fonts failed to load");
		}
		initialized = true;
	}

	void synchronize(const std::vector<RemoteFont>& remoteFonts)
	{
		this->unloadCache();
        std::wstringstream report;
        report << "Synchronization Report\n";
        /// First look for any fonts that should be deleted.
		for (auto file : detail::IterableFontDirectory(this->fontDirectory))
		{
			bool remove = true;
			for (const auto& remote : remoteFonts)
			{
                if (file.path().filename() == remote.getRemoteFile().substr(remote.getRemoteFile().find_last_of("/\\") + 1))
				{
					remove = false;
					break;
				}
			}
			if (remove)
			{
                try
                {
                   // boost::filesystem::remove(file);
                    report << L"Deleted Local Font " << file.path().string().c_str() << '\n';
                }
                catch (const boost::filesystem::filesystem_error& e)
                {
                    /// last effort to preserve some bit of functionality...
                    try
                    {
                        this->updateCache();
                    }
                    catch (...) { /*ignore*/ }

                    /// something else is locking the font file.  abort.
                    throw std::runtime_error(std::string("unable to write to ").append(file.path().filename().string()).append(e.what()));
                }
                
			}
		}

		/// Next, check for updates and new fonts.
		for (const auto& font : remoteFonts)
		{
			std::stringstream ss;
            ss << this->fontDirectory << '\\' << font.getRemoteFile().substr(font.getRemoteFile().find_last_of("/\\") + 1);
			boost::filesystem::path localPath(ss.str());
			if (!boost::filesystem::exists(localPath) || md5(ss.str()) != font.getMD5())
			{
                unsigned int attempts = 0;
                while (!this->shouldStop)
                {
                    try
                    {
                        download(ss.str(), font.getRemoteFile());
                        report << "Updated " << localPath.string().c_str() << '\n';
                        break;
                    }
                    catch (const std::runtime_error& error)
                    {
                        std::wstringstream wss;
                        wss << "error updating " << ss.str().c_str() << " with newer version " << font.getRemoteFile().c_str() << ": " << error.what();
                        if (++attempts >= this->failedDownloadRetryAttempts)
                        {
                            /// Error! Error!
                            wss << " aborting";
                            this->service->WriteEventLogEntry(wss.str().c_str(), EVENTLOG_ERROR_TYPE);
                            break;
                        }
                        else
                        {
                            /// Warning!  Warning!
                            wss << " attempt " << attempts << " of " << this->failedDownloadRetryAttempts;
                            this->service->WriteEventLogEntry(wss.str().c_str(), EVENTLOG_WARNING_TYPE);
                        }
                    }
                }
            }
            else
            {
                report << "Font " << localPath.string().c_str() << " was already up to date\n";
            }
		}
        this->service->WriteEventLogEntry(report.str().c_str(), EVENTLOG_INFORMATION_TYPE);
		this->updateCache();
	}

    FontCacheImpl(const CServiceBase* service, const std::string& fontDirectory, unsigned int failedDownloadRetryDelay, unsigned int failedDownloadRetryAttempts, volatile bool& shouldStop, bool cacheImmediately) :
        service(service), fontDirectory(fontDirectory), failedDownloadRetryDelay(failedDownloadRetryDelay), failedDownloadRetryAttempts(failedDownloadRetryAttempts), shouldStop(shouldStop), initialized(false)
	{
        boost::filesystem::path path(fontDirectory);
		if (!boost::filesystem::exists(path))
		{
            try
            {
                boost::filesystem::create_directory(path);
            }
            catch (const boost::filesystem::filesystem_error& error)
            {
                /// if this happens, we're out of luck.  kill the service & phone home.
                throw std::runtime_error(std::string("cannot create local font directory: ").append(error.what()));
            }
		}
		if (cacheImmediately)
		{
			updateCache();
		}
	}

	~FontCacheImpl()
	{
        /// function can throw, but at this point we really don't care.
        /// everything is unlinked from GDI.  Just wrap the service main method
        /// in an extra try block to suppress the potential for an appcrash.
		unloadCache();
	}

};

FontCache::FontCache(const CServiceBase* service, const std::string& fontDirectory, unsigned int failedDownloadRetryDelay, unsigned int failedDownloadRetryAttempts, volatile bool& shouldStop, bool cacheImmediately) :
impl(new FontCacheImpl(service, fontDirectory, failedDownloadRetryDelay, failedDownloadRetryAttempts, shouldStop, cacheImmediately))
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