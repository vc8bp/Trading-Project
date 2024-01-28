#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
//#include "queue.h"
#include "../includes/tokenMap.hpp"
#include "../includes/queue.h"
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mutex>
#include <condition_variable>


using namespace boost::asio;
using tcp = boost::asio::ip::tcp;

extern volatile bool shouldStop;
extern ThreadSafeQueue<std::string> messageQueue;

extern ThreadSafeMap<std::string> messageMap;


//std::vector<boost::beast::websocket::stream<tcp::socket>> clients;
std::vector<std::unique_ptr<boost::beast::websocket::stream<tcp::socket>>> clients;
std::mutex clietnsMutex;
std::condition_variable clientsCV;

std::unordered_set<int> uniqueTokens;
std::mutex uniqueTokensMutex;
std::condition_variable uniqueTokensCV;

boost::property_tree::ptree parseJson(const std::string& jsonStr);

void readMessagesThread() {
    while (true) {
        if (clients.empty()) continue;

        int i = 0;
        try {
            for (i = 0; i < clients.size(); ++i) {
                boost::beast::flat_buffer buffer;

                boost::system::error_code ec;
                clients[i]->read(buffer, ec);

                if (ec) {
                    std::lock_guard<std::mutex> guard(clietnsMutex);
                    if (clients.size() > i) {
                        std::cerr << "Client Removed from ARRAY" << std::endl;
                        clients.erase(clients.begin() + i);
                        --i;
                    }
                    if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
                        std::cerr << "Client " << i << " disconnected during write: " << ec.message() << std::endl;


                    } else {
                        std::cerr << "Boost.Asio write error: " << ec.message() << std::endl;
                    }
                } else {
                    auto received_message = boost::beast::buffers_to_string(buffer.cdata());
                    std::cout << received_message << std::endl;
                }


            }
        }
        catch (const std::exception& ex) {
            std::cerr << "Caught unknown exception: " << ex.what() << " at client " << i << std::endl;
        }
    }
}

void broadCastFeed() {
    while (true) {
        std::string message = messageQueue.dequeue();
        std::cout<< "DEQUEUED" << std::endl;
        if (clients.empty()) continue;

        std::lock_guard<std::mutex> guard(clietnsMutex);

        int i = 0;
        try {
            for (i = 0; i < clients.size(); i++) {
                std::cout<< "Sending to client : " << i << std::endl;
                
                boost::system::error_code ec;
                clients[i]->write(boost::asio::buffer(message), ec);

                if (ec) {
                    std::lock_guard<std::mutex> guard(clietnsMutex);
                    if (clients.size() > i) {
                        std::cerr << "Client Removed from ARRAY" << std::endl;
                        clients.erase(clients.begin() + i);
                        --i;
                    }
                    if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
                        std::cerr << "Client " << i << " disconnected during write: " << ec.message() << std::endl;
                    } else {
                        std::cerr << "Boost.Asio write error: " << ec.message() << std::endl;
                    }
                }
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "Caught unknown exception: " << ex.what() << " at client " << i << std::endl;
        }
    }
}


int server(const char* ip_address, const char* PORT) {

    auto const address = boost::asio::ip::make_address(ip_address);
    auto const port = static_cast<unsigned short>(std::atoi(PORT));

    boost::asio::io_context ioc{ 1 };

    tcp::acceptor accepter{ ioc, {address, port} };

    std::cout << "WS Server is Listening on ws://" << ip_address << ":" << PORT << std::endl;

    std::thread readThread(readMessagesThread);
    std::thread broadCastThread(broadCastFeed);

    while (true) {
        tcp::socket socket{ ioc };
        accepter.accept(socket);

        std::cout << "Client Connected!! \n";

        std::thread{ [q = std::move(socket)]() mutable {

            boost::beast::websocket::stream<tcp::socket> ws {std::move(q)};

            ws.accept();

            std::string received_message = messageMap.printAll();
            ws.write(boost::asio::buffer(received_message));

            std::lock_guard<std::mutex> guard(clietnsMutex);
            clients.push_back(std::make_unique<boost::beast::websocket::stream<tcp::socket>>(std::move(ws)));

        } }.detach();

    }

    readThread.join();
    broadCastThread.join();

    return 0;
}