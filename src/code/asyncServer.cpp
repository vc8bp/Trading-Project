#include <boost/beast/core.hpp>
#include <boost/bind/bind.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class Listener {
    net::io_context& ioc;
    tcp::acceptor accepter;

public:
    Listener(net::io_context& ioc, unsigned short int port) :
        ioc(ioc), accepter(ioc, { net::ip::make_address("127.0.0.1"), port }) {}

    void asyncAccept() {
        accepter.async_accept(ioc, [&](boost::system::error_code ec, tcp::socket socket) {
            std::cout << "Client Connected" << std::endl;
            asyncAccept();
            });
    }
};


int main() {
    try {
        net::io_context ioc{};
        auto const port = 3001;
        std::make_shared<Listener>(ioc, port)->asyncAccept();

        ioc.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
