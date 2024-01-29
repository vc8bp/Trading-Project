#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <string>
#include <thread>
#include "../includes/queue.h" 
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../includes/tokenMap.hpp"

using namespace boost::asio;
using tcp = boost::asio::ip::tcp;

extern volatile bool shouldStop;
extern ThreadSafeQueue<std::string> messageQueue;

extern ThreadSafeMap<std::string> messageMap;


boost::property_tree::ptree parseJson(const std::string& jsonStr);


int client(const char* ip_address, const char* port) {

    const auto server_address = ip_address;
    const auto server_port = port;

    boost::asio::io_context io_context;

    try {
        //These objects perform our I/O
        tcp::resolver resolver(io_context);
        boost::beast::websocket::stream<tcp::socket> ws{ io_context };

        // Look up the domain name
        auto results = resolver.resolve(server_address, server_port);

        //connecting & handshake
        boost::asio::connect(ws.next_layer(), results);
        ws.handshake(server_address, "/");

        std::cout << "WebSocket handshake successful on ws://" << ip_address << ":" << port << std::endl;

        ws.write(boost::asio::buffer("Hello World fron Client!"));

        while (!shouldStop) {
            boost::beast::flat_buffer buffer;
            try {
                ws.read(buffer);
                auto received_message = boost::beast::buffers_to_string(buffer.cdata());


                if (!received_message.empty()) {
                    boost::property_tree::ptree pt = parseJson(received_message);
                    int token_id = pt.get<int>("token");

                    messageMap.update(token_id, received_message);

                    // messageQueue.enqueue(received_message);

                }

            }
            catch (const boost::beast::system_error& e) {

                std::cerr << "WebSocket error: " << e.what() << std::endl;
                std::cerr << "WebSocket closed by the server." << std::endl;
                shouldStop = true;
                
                break;
            }
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Exception client: " << e.what() << std::endl;
    }

    return 0;
}
