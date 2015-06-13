#ifndef FONT_HPP
#define	FONT_HPP

#include <string>

class Font
{
    std::string name;
    std::string category;
    std::string type;
    std::string remotePath;
    
public:
    Font(const std::string& name, 
         const std::string& category,
         const std::string& type,
         const std::string& remotePath) :
    name(name),
    category(category),
    type(type),
    remotePath(remotePath)
    {
        
    }
    
    const std::string& getName() const
    {
        return this->name;
    }
    
    const std::string& getCategory() const
    {
        return this->category;
    }
    
    const std::string& getType() const
    {
        return this->type;
    }
    
    const std::string& getRemotePath() const
    {
        return this->remotePath;
    }
};

#endif	/* FONT_HPP */

