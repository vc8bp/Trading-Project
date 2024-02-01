#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <string>
#include <thread>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../includes/dataStruct.h"

#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unordered_map>
#include <mutex> // Include the mutex header

using namespace boost::asio;
using tcp = boost::asio::ip::tcp;

extern volatile bool shouldStop;


std::unordered_map<int, SharedData*> myMap;
std::mutex mapMutex; // Mutex to synchronize access to the map


void append(SharedData* data) {
    SharedData* sharedData;
    int askValue = data->ask;
    int price = data->bidqty;

    {
        std::lock_guard<std::mutex> lock(mapMutex); // Lock the map during the critical section

        if (myMap.find(data->token) == myMap.end()) {
            std::cout << "Generating SharedMem for Token : " << data->token << std::endl;

            key_t key = ftok("/home/finrise/project/Trading-Project/feeds/sharedMem", data->token);

            if (key == -1) {
                std::cout<< "error in fork : ";
                perror("ftok");
            }

            if (data->token == 1)
                std::cout << " KEY : " << data->token << " Value : " << price << std::endl;

            int shmid = shmget(key, sizeof(SharedData), 0666 | IPC_CREAT);
            

            if (shmid == -1) {
                std::cerr << "Error creating or getting shared memory segment for token " << data->token << std::endl;
                perror("shmget");
                return;
            }

            sharedData = (SharedData*)shmat(shmid, nullptr, 0);

            if (sharedData == (SharedData*)-1) {
                std::cerr << "Error attaching shared memory for token " << data->token << std::endl;
                return;
            }

            myMap[data->token] = sharedData;
        } else {
            sharedData = myMap[data->token];
        }
    }

    // Assuming SharedData has the same structure as 'data'
    *sharedData = *data;
    // OR: memcpy(sharedData, data, sizeof(SharedData));

    for (const auto& entry : myMap) {
        if (kill(entry.first, SIGUSR1) == -1) {
            perror("kill");
        }
    }
}



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

        SharedData sharedData;

        while (!shouldStop) {
            boost::beast::flat_buffer buffer;
            try {
                ws.read(buffer);
                auto received_message = boost::beast::buffers_to_string(buffer.cdata());

                if (!received_message.empty()) {
                    

                    boost::property_tree::ptree pt = parseJson(received_message);
                    sharedData.token = pt.get<int>("token", 0);
                    sharedData.bid = pt.get<int>("bid", 0);
                    sharedData.bidqty = pt.get<int>("bidqty", 0);
                    sharedData.askqty = pt.get<int>("askqty", 0);
                    sharedData.ask = pt.get<int>("ask", 0);
                    sharedData.ltp = pt.get<int>("ltp", 0);
                    sharedData.ltq = pt.get<int>("ltq", 0);
                    sharedData.ltt = pt.get<int>("ltt", 0);
                    sharedData.exchange_timestamp = pt.get<int>("exchange_timestamp", 0);

                    append(&sharedData);

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
