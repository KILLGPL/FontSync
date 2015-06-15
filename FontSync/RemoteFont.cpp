#include "RemoteFont.hpp"

struct RemoteFont::RemoteFontImpl
{
	std::string md5Hash;

	RemoteFontImpl(const std::string& md5Hash) : md5Hash(md5Hash)
	{

	}
};

RemoteFont::RemoteFont(const std::string& name,
	const std::string& category,
	const std::string& type,
	const std::string& md5Hash) :
	FontBase(name, category, type),
	impl(new RemoteFontImpl(md5Hash))
{

}

RemoteFont::RemoteFont(const RemoteFont& other) : FontBase(other), impl(new RemoteFontImpl(other.impl->md5Hash))
{

}

RemoteFont& RemoteFont::operator=(const RemoteFont& other)
{
	FontBase::operator=(other);
	this->impl->md5Hash = other.impl->md5Hash;
	return *this;
}

const std::string& RemoteFont::getMD5() const {
	return this->impl->md5Hash;
}

RemoteFont::~RemoteFont()
{

}