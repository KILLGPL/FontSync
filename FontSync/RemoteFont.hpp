#ifndef REMOTE_FONT_HPP_INCLUDED
#define REMOTE_FONT_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <memory>
#include <string>
#include "FontBase.hpp"

class RemoteFont : public FontBase
{
	/// Private Implementation
	struct RemoteFontImpl;

	/// Private Implementation
	std::unique_ptr<RemoteFontImpl> impl;

public:

	/**
	* Constructs a RemoteFont with the provided name, category, type, and md5 hash
	*
	* @param name the name of this font
	*
	* @param category the category of this font
	*
	* @param type the type of this font
	*
	*/
	RemoteFont(const std::string& name,
		const std::string& category,
		const std::string& type,
		const std::string& md5Hash);

	/**
	* Copy Constructor
	*
	*/
	RemoteFont(const RemoteFont&);

	/**
	* Copy Assignment
	*
	*/
	RemoteFont& operator=(const RemoteFont&);

	/**
	* Retrieves the MD5 hash of this font
	*
	* @return the MD5 hash of this font
	*
	*/
	virtual const std::string& getMD5() const;

	/**
	* Virtual Destructor
	*
	*/
	virtual ~RemoteFont();
};

#endif
