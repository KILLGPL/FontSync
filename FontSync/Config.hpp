#ifndef CONFIG_HPP_INCLUDED
#define	CONFIG_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <cstdint>
#include <memory>
#include <string>

/**
 * Service configuration.
 * 
 */
class Config {

    /// Private Implementation
    struct ConfigImpl;
    
    /// Private Implementation
    std::unique_ptr<ConfigImpl> impl;
        
public:
    
    /**
     * Retrieves the IP of the sync server
     * 
     * @return the IP of the sync server
     * 
     * @note by default, the sync server is 127.0.0.1
     */
    const std::string& getHost() const;
    
    /**
     * Retrieves the service port of the sync server
     * 
     * @return the service port of the sync server
     * 
     * @note by default, the service port is 80
     */
    uint16_t getPort() const;

    /**
     * Retrieves the number of milliseconds that should elapse between syncs.
     * 
     * @return  the number of milliseconds that should elapse between syncs.
     * 
     * @note by default, there is one minute between syncs
     */
    uint32_t getSyncMillis() const;

	/**
	 * Retreives the resource to request the remote host for
	 *
	 * @return the resource to request the remote host for
	 *
	 * @note by default, the resource is "update.php"
	 *
	 */
	const std::string& getResource() const;

	/**
	 * Retrieves the local directory to install fonts to
	 *
	 * @return the local directory to install fonts to
	 *
	 * @note by default, the local directory is "C:\FontSync\Fonts"
	 */
	const std::string& getLocalFontDirectory() const;

    /**
     * Constructs a configuration object based on the provided INI file
     * 
     * @param configFile the INI configuration file
     * 
     * @throws std::runtime_error if any configuration error occurs
     */
    Config(const std::wstring& configFile);
    
    /**
     * Default constructor
     * Creates a configuration object with the following properties:
     * 
     * host: 127.0.0.1
     * port: 80
     * syncMillis: 60000
     */
    Config();
    
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
