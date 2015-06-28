#ifndef CONFIG_HPP_INCLUDED
#define	CONFIG_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <memory>
#include <string>

/**
 * Application configuration.
 *
 * Accepted configuration file formats are as follows:
 * <ul>
 *  <li>ini</li>
 *  <li>json</li>
 *  <li>xml</li>
 * </ul>
 * 
 */
class Config {

    /// Private Implementation
    struct ConfigImpl;
    
    /// Private Implementation
    std::unique_ptr<ConfigImpl> impl;
        
public:
    
    /**
     * Retrieves the provided configuration option.
     * 
     * @param option the configuration option to retrieve
     *
     * @tparam T the anticipated type of the option data
     *
     * @return the requested configuration option
     *
     */
    template<typename T>
    T get(const std::string& option) const;
    
    /**
     * Constructs a configuration object based on the provided INI file
     * 
     * @param configFile the INI configuration file
     * 
     * @throws std::runtime_error if any configuration error occurs
     */
    Config(const std::string& configFile = "");
    
	/**
	 * Copy Constructor
	 *
	 */
	Config(const Config& other);

	/**
	 * Copy Assignment
	 *
	 */
	Config& operator=(Config& other);

    /**
     * Default destructor (does nothing)
     * 
     * Only explicitly declared because mingw is wonky.
     * 
     */
    ~Config();
};

#endif
