#include "FontCache.hpp"
#include "Logging.hpp"
#include <sstream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

#include <Windows.h>
#include <wingdi.h>


#include "Utilities.hpp"

struct FontCache::FontCacheImpl
{
    /// The directory to use for fonts
	std::string fontDirectory;

    /// The internal cache containing the fonts loaded by GDI
	std::vector<LocalFont> cache;

    /// The delay between retrying a failed download
    unsigned int failedDownloadRetryDelay;

    /// The number of times to try a download before aborting
    unsigned int failedDownloadRetryAttempts;

    void deleteOrphans(const std::vector<RemoteFont>& remoteFonts)
    {
        if (boost::filesystem::exists(getLocalCacheIndexPath()))
        {
            const std::vector<LocalFont>& installedFonts = getManagedFonts(this->fontDirectory);
            for (auto font : installedFonts)
            {
                bool remove = true;
                for (const auto& remote : remoteFonts)
                {
                    if (font.getLocalFile().substr(font.getLocalFile().find_last_of("/\\") + 1) == remote.getRemoteFile().substr(remote.getRemoteFile().find_last_of("/\\") + 1))
                    {
                        remove = false;
                        break;
                    }
                }
                if (remove)
                {
                    try
                    {
                        if (boost::filesystem::exists(font.getLocalFile()))
                        {
                            FONTSYNC_LOG_TRIVIAL(trace) << "Removing orphaned font: " << font.getLocalFile() << "...";
                            int refs = 0;
                            while (RemoveFontResource(font.getLocalFile().c_str()))
                            {
                                refs++;
                            }
                            FONTSYNC_LOG_TRIVIAL(trace) << "Removed " << refs << " references to " << font.getLocalFile() << "...";
                            boost::filesystem::remove(font.getLocalFile());
                            FONTSYNC_LOG_TRIVIAL(trace) << "Deleted local font " << font.getLocalFile() << " from the filesystem...";
                        }
                    }
                    catch (const boost::filesystem::filesystem_error& e)
                    {
                        FONTSYNC_LOG_TRIVIAL(warning) << "Failed to remove orphaned font: " << font.getLocalFile() << "[" << e.what() << "]...";
                    }
                }
            }
        }
    }

    void downloadUpdates(const std::vector<RemoteFont>& remoteFonts)
    {
        for (const auto& font : remoteFonts)
        {
            std::stringstream ss;
            ss << this->fontDirectory << '\\' << font.getRemoteFile().substr(font.getRemoteFile().find_last_of("/\\") + 1);
            boost::filesystem::path localPath(ss.str());
            bool exists = boost::filesystem::exists(localPath);
            bool upToDate = exists && md5(ss.str()) == font.getMD5();
            if (!exists || !upToDate)
            {
                int refs = 0;
                if (exists)
                {
                    while (RemoveFontResource(localPath.string().c_str()))
                    {
                        refs++;
                    }
                    FONTSYNC_LOG_TRIVIAL(trace) << "Removed " << refs << " reference(s) to " << font.getName();
                }

                FONTSYNC_LOG_TRIVIAL(trace) << "Sending WM_FONTCHANGE broadcast...";
                SendMessage(HWND_BROADCAST, WM_FONTCHANGE, NULL, NULL);

                if (!exists)
                {
                    FONTSYNC_LOG_TRIVIAL(trace) << "Downloading new font [" << font.getRemoteFile() << "]...";
                }
                else
                {
                    FONTSYNC_LOG_TRIVIAL(trace) << "Updating existing font [" << font.getRemoteFile() << "]...";
                }
                {
                    unsigned int attempts = 0;
                    do
                    {
                        try
                        {

                            download(ss.str(), font.getRemoteFile());
                            break;
                        }
                        catch (const std::runtime_error& e)
                        {
                            if (++attempts >= this->failedDownloadRetryAttempts)
                            {
                                FONTSYNC_LOG_TRIVIAL(error) << "Failed to download " << font.getRemoteFile() << ": " << e.what() << "\nattempt " << attempts << " of " << this->failedDownloadRetryAttempts;
                                break;
                            }
                            else
                            {
                                FONTSYNC_LOG_TRIVIAL(warning) << "Failed to download " << font.getRemoteFile() << ": " << e.what() << "\nattempt " << attempts << " of " << this->failedDownloadRetryAttempts;
                            }
                        }
                    } while (attempts < this->failedDownloadRetryAttempts);
                }
                if (exists)
                {
                   int restored = refs;
                    while (refs--)
                    {
                        AddFontResource(localPath.string().c_str());
                    }
                    FONTSYNC_LOG_TRIVIAL(trace) << "Restored " << restored << " reference(s) to " << font.getName() << "...";
                    FONTSYNC_LOG_TRIVIAL(trace) << "Sending WM_FONTCHANGE broadcast...";
                    SendMessage(HWND_BROADCAST, WM_FONTCHANGE, NULL, NULL);
                }
            }
            else
            {
                FONTSYNC_LOG_TRIVIAL(trace) << localPath << " was already up to date...";
            }
        }
    }

	void synchronize(const std::vector<RemoteFont>& remoteFonts)
	{
        FONTSYNC_LOG_TRIVIAL(trace) << "Deleting orphaned fonts...";
        this->deleteOrphans(remoteFonts);
	
        FONTSYNC_LOG_TRIVIAL(trace) << "Downloading updates...";
        this->downloadUpdates(remoteFonts);

        FONTSYNC_LOG_TRIVIAL(trace) << "Committing current index...";
        commitAppData();
        
	}

    FontCacheImpl(const std::string& fontDirectory, unsigned int failedDownloadRetryDelay, unsigned int failedDownloadRetryAttempts) :
        fontDirectory(fontDirectory), failedDownloadRetryDelay(failedDownloadRetryDelay), failedDownloadRetryAttempts(failedDownloadRetryAttempts)
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
                throw std::runtime_error(std::string("cannot create local font directory: ").append(error.what()));
            }
		}
        if (boost::filesystem::exists(getLocalCacheIndexPath()))
        {
            for (auto font : getManagedFonts(this->fontDirectory))
            {
                if (AddFontResourceA(font.getLocalFile().c_str()) > 0)
                {
                    cache.push_back(font);
                }
                else
                {
                    FONTSYNC_LOG_TRIVIAL(warning) << "Failed to load managed font: " << font.getLocalFile() << "[" << errorString(GetLastError()) << "]...";
                }
            }
        }
	}

	~FontCacheImpl()
	{
        FONTSYNC_LOG_TRIVIAL(trace) << "~FontCacheImpl()";
        for (auto font : getManagedFonts(this->fontDirectory))
        {
            if (RemoveFontResourceA(font.getLocalFile().c_str()) == 0)
            {
                FONTSYNC_LOG_TRIVIAL(warning) << "Failed to unload managed font: " << font.getLocalFile() << "[" << errorString(GetLastError()) << "]...";
            }
        }
	}

};

FontCache::FontCache(const std::string& fontDirectory, unsigned int failedDownloadRetryDelay, unsigned int failedDownloadRetryAttempts) :
impl(new FontCacheImpl(fontDirectory, failedDownloadRetryDelay, failedDownloadRetryAttempts))
{

}

void FontCache::synchronize(const std::vector<RemoteFont>& remoteFonts)
{
	this->impl->synchronize(remoteFonts);
}

FontCache::~FontCache()
{

}