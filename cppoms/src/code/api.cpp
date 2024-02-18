#include "../includes/certs.hpp"
#include "../includes/verbs.hpp"
#include "../includes/api.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

resT fetch(const char* endpoint, const char* method, bool isSsl){
    try {
        auto [host, target] = parseEndpoint(endpoint);

        auto const port = isSsl ? "443" : "80";
        int version = 11;

        boost::asio::io_context ioc;

        ssl::context ctx{ssl::context::sslv23_client};

        load_root_certificates(ctx);

        tcp::resolver resolver{ioc};
        ssl::stream<tcp::socket> stream{ioc, ctx};

        if(! SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw boost::system::system_error{ec};
        }

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        boost::asio::connect(stream.next_layer(), results.begin(), results.end());

        // Perform the SSL handshake
        stream.handshake(ssl::stream_base::client);

        // Set up an HTTPrequest message
        http::request<http::string_body> req{convert_method_to_enum(method), target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // if(std::strcmp(method, "POST")){
            req.set(http::field::content_type, "application/json"); 
            std::string jsonBody = R"({"email": "d","password": "d"})";
            req.body() = jsonBody;
            req.prepare_payload();
        // }

        http::write(stream, req);

        boost::beast::flat_buffer buffer;
        resT res;

        http::read(stream, buffer, res);

        std::cout << res << std::endl;

        boost::system::error_code ec;
        stream.next_layer().close(ec);
        
        if(ec == boost::asio::error::eof) ec.assign(0, ec.category()); 
        if(ec) throw boost::system::system_error{ec};

        return res;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}
