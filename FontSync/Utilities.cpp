#include "Utilities.hpp"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <wininet.h>
#include <urlmon.h>
#include <comdef.h>
#include <Shlobj.h>
#include <Shlwapi.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>

void WriteEventLogEntry(const wchar_t* pszMessage)
{
    std::wcout << pszMessage << std::endl;
}

void WriteEventLogEntry(const char* pszMessage)
{
    std::wcout << pszMessage << std::endl;
}

std::wstring errorString(DWORD errorCode)
{
	if (errorCode == 0)
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

std::string md5(const std::string& file)
{
	try
	{
		CryptoPP::Weak::MD5 hash;
		byte buffer[2 * CryptoPP::Weak::MD5::DIGESTSIZE];

		CryptoPP::FileSource f(file.c_str(), true,
			new CryptoPP::HashFilter(hash,
			new CryptoPP::HexEncoder(new CryptoPP::ArraySink(buffer, 2 * CryptoPP::Weak::MD5::DIGESTSIZE))));
		return std::string((const char*)buffer, 2 * CryptoPP::Weak::MD5::DIGESTSIZE);
	}
	catch (const std::exception& error)
	{
		throw std::runtime_error(error.what());
	}
}

void download(const std::string& writeTo, const std::string& readFrom)
{
	DeleteUrlCacheEntry(readFrom.c_str());

	HRESULT hr = URLDownloadToFile(NULL,
		                           readFrom.c_str(),
    		                       writeTo.c_str(),
		                           0,
		                           NULL);
	if (!SUCCEEDED(hr))
	{
		throw std::runtime_error("error downloading file");
	}
}

void initAppData(const std::string& json)
{
    try
    {
        CHAR path[MAX_PATH];
        HRESULT result;
        if ((result = SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)) == S_OK)
        {
            PathAppendA(path, "FontSync");
            if (!boost::filesystem::exists(path))
            {
                boost::filesystem::create_directories(path);
            }
            PathAppendA(path, "local_cache_temp.json");
        }
        else
        {
            throw std::exception(_com_error(result).ErrorMessage());
        }
        boost::property_tree::ptree tree;
        {
            std::stringstream ss;
            ss << json;
            boost::property_tree::json_parser::read_json(ss, tree);
        }
        boost::property_tree::json_parser::write_json(std::string(path), tree);
    }
    catch (...)
    {
        throw std::runtime_error("unable to save temporary local cache");
    }
}

std::string getLocalCacheIndexPath()
{
    CHAR path[MAX_PATH];
    HRESULT result;
    if ((result = SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)) == S_OK)
    {
        PathAppendA(path, "FontSync\\local_cache.json");
        return path;
    }
    else
    {
        throw std::exception(_com_error(result).ErrorMessage());
    }
}

void commitAppData()
{
    CHAR temp[MAX_PATH], perm[MAX_PATH];
    HRESULT result;
    if ((result = SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, temp)) == S_OK)
    {
        memcpy(perm, temp, MAX_PATH);
        PathAppendA(temp, "FontSync\\local_cache_temp.json");
        PathAppendA(perm, "FontSync\\local_cache.json");
        DeleteFile(perm);
        rename(temp, perm);
    }
    else
    {
        throw std::runtime_error(_com_error(result).ErrorMessage());
    }
}

std::vector<LocalFont> getManagedFonts(const std::string& fontDirectory)
{
    boost::property_tree::ptree tree;
    CHAR path[MAX_PATH];
    HRESULT result;
    if ((result = SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)) == S_OK)
    {
        PathAppendA(path, "FontSync\\local_cache.json");
        boost::property_tree::json_parser::read_json(path, tree);
    }
    else
    {
        throw std::exception(_com_error(result).ErrorMessage());
    }

    std::vector<LocalFont> rv;

    for (auto font : tree)
    {
        std::stringstream ss;
        ss << fontDirectory << '\\' << font.second.get_child("remote_file").data().substr(font.second.get_child("remote_file").data().find_last_of("/\\") + 1);
        boost::filesystem::path localPath(ss.str());
        if (boost::filesystem::exists(localPath))
        {
            rv.push_back(LocalFont(
                font.second.get_child("name").data(),
                font.second.get_child("category").data(),
                font.second.get_child("type").data(),
                ss.str()
                ));
            std::cout << "Loaded Font " << ss.str() << std::endl;
        }
    }
    return rv;
}