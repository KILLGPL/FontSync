#ifndef FONT_BASE_HPP_INCLUDED
#define	FONT_BASE_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <memory>
#include <string>

/**
 * Parent class to both local and remote fonts.
 *
 */
class FontBase
{
	/// Private Implementation
	struct FontBaseImpl;

	/// Private Implementation
	std::unique_ptr<FontBaseImpl> impl;
    
protected:
	
	/**
	 * Constructs a FontBase with the provided name, category, and type
	 * 
	 * @param name the name of this font
	 *
	 * @param category the category of this font
	 *
	 * @param type the type of this font
	 *
	 */
	FontBase(const std::string& name, 
		     const std::string& category, 
			 const std::string& type);

public:
	
	/**
	 * Copy Constructor
	 *
	 */
	FontBase(const FontBase&);

	/**
	 * Copy Assignment
	 *
	 */
	FontBase& operator=(const FontBase&);

	/**
	* Retrieves the name of this font
	*
	* @return the name of this font
	*
	*/
	const std::string& getName() const;
	
	/**
	* Retrieves the category of this font
	*
	* @return the category of this font
	*
	*/
	const std::string& getCategory() const;

	/**
	* Retrieves the type of this font
	*
	* @return the type of this font
	*
	*/
	const std::string& getType() const;

	/**
	 * Retrieves the MD5 hash of this font
	 * 
	 * @return the MD5 hash of this font
	 *
	 */
	virtual const std::string& getMD5() const = 0;

	/**
	 * Virtual Destructor
	 *
	 */
	virtual ~FontBase();
};

#endif
