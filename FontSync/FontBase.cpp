#include "FontBase.hpp"

struct FontBase::FontBaseImpl
{
	std::string name;
	std::string category;
	std::string type;

	FontBaseImpl(const std::string& name,
		         const std::string& category,
		         const std::string& type) :
		name(name),
		category(category),
		type(type)
	{

	}
};

FontBase::FontBase(const std::string& name,
	               const std::string& category,
    	           const std::string& type) : 
	impl(new FontBaseImpl(name, category, type))
{

}

FontBase::FontBase(const FontBase& other) : impl(new FontBaseImpl(other.impl->name,
											                      other.impl->category,
																  other.impl->type))
{

}

FontBase& FontBase::operator=(const FontBase& other)
{
	this->impl->name = other.impl->name;
	this->impl->category = other.impl->category;
	this->impl->type = other.impl->type;
	return *this;
}

const std::string& FontBase::getName() const
{
	return this->impl->name;
}

const std::string& FontBase::getCategory() const
{
	return this->impl->category;
}

const std::string& FontBase::getType() const
{
	return this->impl->type;
}

FontBase::~FontBase()
{

}