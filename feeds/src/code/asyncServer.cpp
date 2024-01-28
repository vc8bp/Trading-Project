#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <thread>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "../includes/tokenMap.hpp"
#include "../includes/queue.h"
#include "../includes/asyncServer.h"

extern volatile bool shouldStop;
extern ThreadSafeQueue<std::string> messageQueue;

boost::property_tree::ptree parseJson(const std::string& jsonStr);

int extractToken(const std::string& jsonString) {
    // Find the position of "token" in the JSON string
    size_t tokenPos = jsonString.find("\"token\"");

    if (tokenPos != std::string::npos) {
        // Find the position of the colon after "token"
        size_t colonPos = jsonString.find(":", tokenPos);

        if (colonPos != std::string::npos) {
            // Find the position of the comma after the "token" value
            size_t commaPos = jsonString.find(",", colonPos);

            if (commaPos != std::string::npos) {
                // Extract the "token" value substring and convert it to an integer
                std::string tokenStr = jsonString.substr(colonPos + 1, commaPos - colonPos - 1);
                return std::stoi(tokenStr);
            }
        }
    }

    // Return -1 if the "token" field is not found
    return -1;
}


namespace beast = boost::beast;
namespace http  = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;



MyWebsocket::MyWebsocket(tcp::socket&& socket, std::shared_ptr<Listener> listener) :
    ws(std::move(socket)),
    listener(listener) {}

void MyWebsocket::run() {
    ws.async_accept([self{shared_from_this()}](beast::error_code ec) {
        if (ec) {
            std::cout << "Error on run" << ec.message() << std::endl;
            return;
        }
        self->listener->addClient(self);
        self->echo();
    });
}

void MyWebsocket::echo() {
    ws.async_read(buffer, [self{shared_from_this()}](beast::error_code ec, std::size_t bytes_recieved) {
        if (ec == websocket::error::closed) {
            self->listener->removeClient(self);
            return;
        }
        if (ec) {
            std::cout << "Error on Read : " << ec.message() << std::endl;
            return;
        }

        auto out = beast::buffers_to_string(self->buffer.cdata());

        try {
            int parsedValue = std::stoi(out);
            self->subscribe(parsedValue);
        } catch (const std::invalid_argument& e) {
            std::cout << "Error: " << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }

        // self->listener->broadcastMessage(out);
        self->buffer.consume(self->buffer.size());
        self->echo();
    });
}

int MyWebsocket::isSubscribed(int token){
    return this->tokensSubscribed.find(token) != this->tokensSubscribed.end();
}
void MyWebsocket::subscribe(int token){
    this->tokensSubscribed.insert(token);
}

Listener::Listener(net::io_context& ioc, const char* ip , int port) :
    ioc(ioc),
    acceptor(net::make_strand(ioc), {tcp::endpoint(tcp::v4(), port)}) {}

void Listener::asyncAccept() {
    acceptor.async_accept(net::make_strand(ioc), [self{shared_from_this()}](boost::system::error_code ec, tcp::socket socket) {
        std::make_shared<MyWebsocket>(std::move(socket), self)->run();
        std::cout << "Client accepted!!" << std::endl;
        self->asyncAccept();
    });
}

void Listener::addClient(std::shared_ptr<MyWebsocket> client) {
    clients.push_back(client);
}

void Listener::removeClient(std::shared_ptr<MyWebsocket> client) {
    std::cout << "client Disconnected" << std::endl;
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
}

websocket::stream<beast::tcp_stream>& MyWebsocket::getWebSocket() {
    return ws;
}

void Listener::broadcastMessage() {
    std::thread([this]() {
        while (true) {
            std::cout << messageQueue.size() <<std::endl;
            std::string message = messageQueue.dequeue();

            if (!clients.size()) continue;

            std::unique_lock<std::mutex> ul(mutex);

            this->pendingWrites = clients.size(); 
            ul.unlock(); 

            int token_id = extractToken(message);
            // boost::property_tree::ptree pt = parseJson(message);
            // int token_id = pt.get<int>("token");

            for (auto& client : clients) {
                // if(client->isSubscribed(token_id)){
                if(true){    
                    client->getWebSocket().async_write(net::buffer(message), [self{shared_from_this()}, client](beast::error_code ec, std::size_t) {
                        std::lock_guard<std::mutex> lg(self->mutex);
                        self->pendingWrites -= 1;

                        if (ec == websocket::error::closed) {
                            self->removeClient(client);
                        }
                        if (self->pendingWrites == 0) {
                            self->condition.notify_one();
                        }
                    });
                } else {
                    std::lock_guard<std::mutex> lg(this->mutex);
                    this->pendingWrites -= 1;

                    if (this->pendingWrites == 0) {
                        this->condition.notify_one();
                    }
                }

            }

            ul.lock();
            condition.wait(ul, [this] { return this->pendingWrites == 0; });
        }
    }).detach();
}



// void Listener::broadcastMessage() {
//     int threads = std::thread::hardware_concurrency();
//     for (int i = 0; i < threads; ++i) {
//         broadcastThreads.emplace_back([this]() {
//             while (true) {
//                 std::cout << messageQueue.size() <<std::endl;    
//                 std::string message = messageQueue.dequeue();

//                 if (!clients.size()) continue;

//                 std::unique_lock<std::mutex> ul(mutex);

//                 this->pendingWrites = clients.size(); 
//                 ul.unlock(); 

//                 boost::property_tree::ptree pt = parseJson(message);
//                 int token_id = pt.get<int>("token");
//                 for (auto& client : clients) {           
//                     if (client->isSubscribed(token_id)) {
//                         client->getWebSocket().async_write(net::buffer(message), [self{shared_from_this()}, client](beast::error_code ec, std::size_t) {
//                             std::lock_guard<std::mutex> lg(self->mutex);
//                             self->pendingWrites -= 1;
                            
//                             if (ec == websocket::error::closed) {
//                                 self->removeClient(client);
//                             }
//                             if (self->pendingWrites == 0) {
//                                 self->condition.notify_one();
//                             }
//                         });
//                     }
//                 }
//                 ul.lock();
//                 condition.wait(ul, [this] { return this->pendingWrites == 0; });
//             }
//         }).detach();
//     }
// }


void Listener::stopMessageBroadcast() {
    for (auto& thread : broadcastThreads) {
        thread.join();
    }
    broadcastThreads.clear();
}

int asyncServer(const char* ip_address, int port){
    std::cout<< "Running : " << std::endl;
    int threads = 4;
    net::io_context ioc{threads};

    std::shared_ptr<Listener> listener = std::make_shared<Listener>(ioc, ip_address ,port);
    std::cout << "Server is Listening on : " << ip_address << ":" << port << std::endl;

    listener->asyncAccept();
    listener->broadcastMessage();

    std::vector<std::thread> v;
    v.reserve(threads-1);

    for(auto i = threads -1; i > 0; --i){
        std::cout<< "Booting Thread : " << i << std::endl;
        v.emplace_back([&ioc](){ioc.run();});
    }   
    ioc.run();

    listener->stopMessageBroadcast();

    return 0;
}