#ifndef UTILITIES_HPP_INCLUDED
#define UITILITES_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/**
* Retrieves the error associated with the provided error code
*
* @param errorCode the error code to look up
*
* @return the error message of the provided error code
*
*/
std::wstring errorString(DWORD errorCode);

/**
 * Calculates the MD5 hash of the provided file
 * 
 * @param file the file to hash
 *
 * @return the MD5 hash of the provided file
 *
 * @throws std::runtime_error if any hashing error occurs
 *
 */
std::string md5(const std::string& file);

#endif
