#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include "queue.h"
#include <fstream>

using namespace boost::asio;
using tcp = boost::asio::ip::tcp;

extern volatile bool shouldStop;
extern ThreadSafeQueue<std::string> messageQueue;

std::vector<boost::beast::websocket::stream<tcp::socket>> clients;

//opening file to Store data
std::ofstream outFile("feeds.txt", std::ios::app);



int server(const char* ip_address, const char* PORT) {
    int isFileOpen = true;

    if (!outFile.is_open()) {
        isFileOpen = false;
        std::cout << "Failed To open file!!" << std::endl;
    }


    auto const address = boost::asio::ip::make_address(ip_address);
    auto const port = static_cast<unsigned short>(std::atoi(PORT));

    boost::asio::io_context ioc{ 1 };

    tcp::acceptor accepter{ ioc, {address, port} };

    std::cout << "WS Server is Listening on ws://" << ip_address << ":" << PORT << std::endl;

    while (true) {
        tcp::socket socket{ ioc };
        accepter.accept(socket);

        std::cout << "Client Connected!! \n";

        std::thread{ [q = std::move(socket)]() mutable {

            boost::beast::websocket::stream<tcp::socket> ws {std::move(q)};

            ws.accept();
            clients.push_back(std::move(ws));

            while (true) {
                try {
                    std::string received_message = messageQueue.dequeue();

                    outFile << received_message << "\n";

                    // Broadcast the received message to all connected clients
                    for (auto& client : clients) {
                        client.write(boost::asio::buffer(received_message));
                    }
                    
                } catch (const boost::beast::system_error& e) {
                      if (e.code() != boost::beast::websocket::error::closed) {
                          std::cout << e.code().message() << std::endl;   
                      }
                      std::cout << "Client Disconnected!!" << std::endl;
                      outFile.close();
                      break;
                } catch (const std::exception& e) {
                     std::cerr << "Exception: " << e.what() << std::endl;
                     std::cout << "Client Disconnected!!" << std::endl;
                     outFile.close();
                     break;
                }
            }


        } }.detach();

    }

    return 0;
}
