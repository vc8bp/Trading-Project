#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <unordered_set>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Listener;

class MyWebsocket : public std::enable_shared_from_this<MyWebsocket> {
    websocket::stream<beast::tcp_stream> ws;
    beast::flat_buffer buffer;
    std::shared_ptr<Listener> listener;
    std::unordered_set<int> tokensSubscribed;

public:
    MyWebsocket(tcp::socket&& socket, std::shared_ptr<Listener> listener);

    void run();

    void echo();
    int isSubscribed(int token);
    void subscribe(int token);

    websocket::stream<beast::tcp_stream>& getWebSocket();
    void writeSync(const std::string& message) ;
};

class Listener : public std::enable_shared_from_this<Listener> {
    net::io_context& ioc;
    tcp::acceptor acceptor;
    std::vector<std::shared_ptr<MyWebsocket>> clients;
    std::vector<std::thread> broadcastThreads;

public:
    Listener(net::io_context& ioc, const char* ip ,int port);

    void asyncAccept();
    void addClient(std::shared_ptr<MyWebsocket> client);
    void removeClient(std::shared_ptr<MyWebsocket> client);
    void broadcastMessage();
    void stopMessageBroadcast();
};
