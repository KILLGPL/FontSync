#ifndef LOCAL_FONT_HPP_INCLUDED
#define LOCAL_FONT_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <memory>
#include "FontBase.hpp"

class LocalFont : public FontBase
{
	/// Private Implementation
	struct LocalFontImpl;

	/// Private Implementation
	std::unique_ptr<LocalFontImpl> impl;

public:

	/**
	* Constructs a LocalFont with the provided name, category, type, and md5 hash
	*
	* @param name the name of this font
	*
	* @param category the category of this font
	*
	* @param type the type of this font
	*
	*/
	LocalFont(const std::string& name,
		const std::string& category,
		const std::string& type,
		const std::string& localFile);

	/**
	* Copy Constructor
	*
	*/
	LocalFont(const LocalFont&);

	/**
	* Copy Assignment
	*
	*/
	LocalFont& operator=(const LocalFont&);

	/**
	* Retrieves the MD5 hash of this font
	*
	* @return the MD5 hash of this font
	*
	*/
	virtual const std::string& getMD5() const;

	/**
	 * Retrieves the local file that holds this font
	 *
	 * @return the local file that holds this font
	 *
	 */
	const std::string& getLocalFile() const;

	/**
	* Virtual Destructor
	*
	*/
	virtual ~LocalFont();
};

#endif
