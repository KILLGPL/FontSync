#ifndef UPDATE_RECEIVER_HPP_INCLUDED
#define UPDATE_RECEIVER_HPP_INCLUDED

/// some microsoft compilers still benefit from the use of #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <memory>
#include <string>
#include <vector>
#include "RemoteFont.hpp"

/**
 * A networking helper to retrieve the current remote font index
 *
 */
class UpdateReceiver
{
	/// Private Implementation
	struct UpdateReceiverImpl;

	/// Private Implementation
	std::unique_ptr<UpdateReceiverImpl> impl;

public:

	/**
	 * Constructs an UpdateReceiver with the provided host, port, and resource string.
	 *
	 * @param host the IP address of the machine that hosts the update server
	 *
	 * @param port the port that the update server is listening on
	 *
	 * @param resource the resource string to use in order to request an updated index
	 *
	 */
	UpdateReceiver(const std::string& host, uint16_t port, const std::string& resource);
    std::string readJSON();
	/**
	 * Retrieves the current remote font index from the update server
	 *
	 * @return the current remote font index from the update server
	 *
	 * @throws std::runtime_error if any error occurs
	 *
	 */
	std::vector<RemoteFont> getRemoteFontIndex();

	/**
	 * Default Destructor
	 *
	 */
	~UpdateReceiver();
};

#endif
