#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <iostream>
#include <thread>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../includes/queue.h"
#include "../includes/tokenMap.hpp"


boost::property_tree::ptree readJsonFromFile(const std::string& filename) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(filename, pt);
    return pt;
}

boost::property_tree::ptree parseJson(const std::string& jsonStr) {
    boost::property_tree::ptree pt;
    std::istringstream is(jsonStr);
    boost::property_tree::read_json(is, pt);
    return pt;
}


volatile bool shouldStop = false;
ThreadSafeQueue<std::string> messageQueue;

ThreadSafeMap<std::string> messageMap;

int client(const char* ip_address, const char* port);
int server(const char* ip_address, const char* port);
int asyncServer(const char* ip_address, const char* PORT);


int main() {
    boost::property_tree::ptree pt = readJsonFromFile("info.json");

    std::string serverIp = pt.get<std::string>("server.ip");
    std::string serverPort = pt.get<std::string>("server.port");
    std::string clientIp = pt.get<std::string>("client.ip");
    std::string clientPort = pt.get<std::string>("client.port");

    // Convert string const char* for function calls
    const char* serverIpCStr = serverIp.c_str();
    const char* serverPortCStr = serverPort.c_str();
    const char* clientIpCStr = clientIp.c_str();
    const char* clientPortCStr = clientPort.c_str();

    std::thread clientThread(client, clientIpCStr, clientPortCStr);
    //std::thread serverThread(asyncServer, serverIpCStr, serverPortCStr);
    std::thread serverThread(server, serverIpCStr, serverPortCStr);

    // Wait for both threads to finish
    clientThread.join();
    serverThread.join();

    return 0;
}

