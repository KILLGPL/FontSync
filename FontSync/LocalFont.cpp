#include "LocalFont.hpp"

#include "Utilities.hpp"

struct LocalFont::LocalFontImpl
{
	std::string localFile;
	std::string md5Hash;

	LocalFontImpl(const std::string& localFile) : localFile(localFile), md5Hash(md5(localFile))
	{

	}

	LocalFontImpl(const std::string& localFile, const std::string& md5Hash) : localFile(localFile), md5Hash(md5Hash)
	{

	}
};

LocalFont::LocalFont(const std::string& name,
	const std::string& category,
	const std::string& type,
	const std::string& localFile) :
	FontBase(name, category, type),
	impl(new LocalFontImpl(localFile))
{

}

LocalFont::LocalFont(const LocalFont& other) : FontBase(other), impl(new LocalFontImpl(other.impl->localFile, other.impl->md5Hash))
{

}

LocalFont& LocalFont::operator=(const LocalFont& other)
{
	FontBase::operator=(other);
	this->impl->md5Hash = other.impl->md5Hash;
	return *this;
}

const std::string& LocalFont::getMD5() const
{
	return this->impl->md5Hash;
}

const std::string& LocalFont::getLocalFile() const
{
	return this->impl->localFile;
}

LocalFont::~LocalFont()
{

}