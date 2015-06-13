#ifndef CLIENT_HPP
#define	CLIENT_HPP

#include <boost/asio.hpp>

#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Font.hpp"

class SyncClient
{
    std::string host;
    uint16_t port;
    
    void createRequest(boost::asio::streambuf& request)
    {
        std::ostream stream(&request);
        stream << "GET /json.php HTTP/1.0\r\n";
        stream << "Host: localhost\r\n";
        stream << "Accept: */*\r\n";
        stream << "Connection: close\r\n\r\n";
    }

    void validate(std::istream& stream)
    {
        std::string httpVersion;
        uint16_t statusCode;
        stream >> httpVersion 
               >> statusCode;
        if(httpVersion.substr(0, 5) != "HTTP/")
        {
            throw std::runtime_error("invalid response");
            /// Invalid response
        }
        else if(statusCode != 200)
        {
            throw std::runtime_error("http response code " + std::to_string(statusCode));
        }
    }
    
    std::string readJson(boost::asio::io_service& service)
    {
        /// create a tcp socket and connect to the sync server
        boost::asio::ip::tcp::socket socket(service);
        socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(this->host), this->port));

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
            while(std::getline(stream, dummy) && dummy != "\r");
        }

        /// read the response data
        std::stringstream json;
        boost::system::error_code ec;
        
        if (response.size() > 0)
        {
            json << &response;
        }
        
        while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), ec))
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
    
public:
    
    SyncClient(const std::string& host, uint16_t port) : host(host), port(port)
    {
        
    }
    
    /**
     * Attempts to retrieve a list of all tracked fonts from the remote host.
     * 
     * @param service the {@link boost::asio::io_service} that should manage the I/O
     * 
     * @return a list of all tracked fonts read from the remote host
     * 
     * @throws boost::asio::system_error if an I/O error occurs
     * 
     * @throws std::runtime_error if the server sends an invalid response or 
     *         returns a status code other than 200
     * 
     * @throws boost::property_tree::json_parser_error if a parsing error 
     *         occurs
     */
    std::vector<Font> fetchTrackedFonts(boost::asio::io_service& service)
    {
        /// create an input stream from the json read from the sync server
        std::istringstream iss(readJson(service));
        
        /// populate a property tree based on the json
        boost::property_tree::ptree tree;
        boost::property_tree::json_parser::read_json(iss, tree);
        
        /// load up some easy to use font objects to return to the caller
        std::vector<Font> fonts;
        for(auto font : tree)
        {
            fonts.push_back(Font(
                font.second.get_child("name").data(),
                font.second.get_child("category").data(),
                font.second.get_child("type").data(),
                font.second.get_child("path").data()
            ));
        }
        return fonts;
    }
};

#endif	/* CLIENT_HPP */
