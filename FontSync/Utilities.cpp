#include "Utilities.hpp"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <wininet.h>
#include <urlmon.h>

#include <boost/filesystem.hpp>

#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>

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
		throw new std::runtime_error("error downloading file");
	}
}