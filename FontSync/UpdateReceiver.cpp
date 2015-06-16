#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "RemoteFont.hpp"
#include "UpdateReceiver.hpp"
struct UpdateReceiver::UpdateReceiverImpl
{
	boost::asio::io_service service;
	std::string host;
	uint16_t port;
	std::string resource;

	void createRequest(boost::asio::streambuf& request)
	{
		std::ostream stream(&request);
		stream << "GET / " << this->resource << " HTTP/1.0\r\n";
		stream << "Host: " << this->host << "\r\n";
		stream << "Accept: */*\r\n";
		stream << "Connection: close\r\n\r\n";
	}

	void validate(std::istream& stream)
	{
		std::string httpVersion;
		uint16_t statusCode;
		stream >> httpVersion
			>> statusCode;
		if (httpVersion.substr(0, 5) != "HTTP/")
		{
			throw std::runtime_error("invalid response");
			/// Invalid response
		}
		else if (statusCode != 200)
		{
			throw std::runtime_error("http response code " + std::to_string(statusCode));
		}
	}

	std::string readJson()
	{
		/// create a tcp socket and connect to the sync server
		boost::asio::ip::tcp::socket socket(service);
		socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(host), port));

		/// send request
		{
			boost::asio::streambuf request;
			createRequest(request);
			boost::asio::write(socket, request);
		}

		/// set up our receive buffer & stream
		boost::asio::streambuf response;
		std::istream stream(&response);

		/// read & validate the http version & status code
		boost::asio::read_until(socket, response, "\r\n");
		validate(stream);

		/// ignore the response headers
		boost::asio::read_until(socket, response, "\r\n\r\n");
		{
			std::string dummy;
			while (std::getline(stream, dummy) && dummy != "\r");
		}

		/// read the response data
		std::stringstream json;
		boost::system::error_code ec;

		if (response.size() > 0)
		{
			json << &response;
		}

		while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), ec))
		{
			json << &response;
		}

		/// gracefully kill our socket (without caring if it failed)
		{
			boost::system::error_code ignored;
			socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
			socket.close(ignored);
		}
		/// if we hit an error before reaching EOF, throw
		if (ec != boost::asio::error::eof)
		{
			throw boost::system::system_error(ec);
		}
		return json.str();
	}

	UpdateReceiverImpl(const std::string& host, uint16_t port, const std::string& resource) :
		host(host), port(port), resource(resource)
	{

	}
};

UpdateReceiver::UpdateReceiver(const std::string& host, uint16_t port, const std::string& resource) :
	impl(new UpdateReceiverImpl(host, port, resource))
{

}

std::vector<RemoteFont> UpdateReceiver::getRemoteFontIndex()
{
	/// create an input stream from the json read from the sync server
	std::istringstream iss(this->impl->readJson());

	/// populate a property tree based on the json
	boost::property_tree::ptree tree;
	boost::property_tree::json_parser::read_json(iss, tree);

	/// load up some easy to use font objects to return to the caller
	std::vector<RemoteFont> remoteFonts;
	for (auto font : tree)
	{
		remoteFonts.push_back(RemoteFont(
		font.second.get_child("name").data(),
		font.second.get_child("category").data(),
		font.second.get_child("type").data(),
		font.second.get_child("remote_file").data(),
		font.second.get_child("md5").data()));
	}
	return remoteFonts;
}

UpdateReceiver::~UpdateReceiver()
{

}
