#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
//#include "queue.h"
#include "tokenMap.hpp"
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include "queue.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mutex>
#include <condition_variable>


using namespace boost::asio;
using tcp = boost::asio::ip::tcp;

extern volatile bool shouldStop;
extern ThreadSafeQueue<std::string> messageQueue;

extern ThreadSafeMap<std::string> messageMap;


std::vector<boost::beast::websocket::stream<tcp::socket>> clients;

std::unordered_set<int> uniqueTokens;
std::mutex uniqueTokensMutex;
std::condition_variable uniqueTokensCV;

boost::property_tree::ptree parseJson(const std::string& jsonStr);



void readMessagesThread() {
    while (!shouldStop) {
        try {
            // Read messages from clients and push them to the queue
            for (size_t i = 0; i < clients.size(); ++i) {
                boost::beast::flat_buffer buffer;
                clients[i].read(buffer);
                auto received_message = boost::beast::buffers_to_string(buffer.cdata());

                //int token = std::stoi(received_message);

                //std::cout << token << std::endl;

                //{
                //    std::lock_guard<std::mutex> lock(uniqueTokensMutex);
                //    uniqueTokens.insert(token);
                //}
            }
        }
        catch (const boost::beast::system_error& e) {
            if (e.code() != boost::beast::websocket::error::closed) {
                std::cout << e.code().message() << std::endl;
            }
            std::cout << "Client Disconnected!!" << std::endl;
            break;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception server: " << e.what() << std::endl;
            std::cout << "Client Disconnected!!" << std::endl;
            break;
        }
    }
}

void broadCastFeed() {
    while (!shouldStop) {
        try {
            std::string message = messageQueue.dequeue();
            for (auto& client : clients) {
                client.write(boost::asio::buffer(message));
                //boost::property_tree::ptree pt = parseJson(message);
                //int token_id = pt.get<int>("token");

                //{
                //    std::unique_lock<std::mutex> lock(uniqueTokensMutex);
                //    if (uniqueTokens.find(token_id) != uniqueTokens.end()) {
                //        client.write(boost::asio::buffer(message));
                //    }
                //}

            }

        }
        catch (const boost::beast::system_error& e) {
            if (e.code() != boost::beast::websocket::error::closed) {
                std::cout << e.code().message() << std::endl;
            }
            std::cout << "Client Disconnected!!" << std::endl;
            break;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception server: " << e.what() << std::endl;
            std::cout << "Client Disconnected!!" << std::endl;
            break;
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
            clients.push_back(std::move(ws));

            for (const auto& token : uniqueTokens) {
                std::string received_message = messageMap.get(token);
                ws.write(boost::asio::buffer(received_message));
            }

            while (true) {
                
            }
        } }.detach();

    }

    return 0;
}
