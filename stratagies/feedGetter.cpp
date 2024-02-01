#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cerrno>
#include <cstring>
#include <signal.h>

struct SharedData {
    int token;
    int bid;
    int bidqty;
    int askqty;
    int ask;
    int ltp;
    int ltq;
    int ltt;
    int exchange_timestamp;
};
void signalHandler(int signum) {
    if (signum == SIGUSR1) {
        std::cout<< "Changed" << std::endl;
    }
}

int getFeed(int token) {

    key_t key = ftok("/home/finrise/project/Trading-Project/feeds/sharedMem", token);
    
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    int shmid = shmget(key, sizeof(SharedData), 0666);

    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    signal(SIGUSR1, signalHandler);

    struct SharedData *sharedData = (struct SharedData *)shmat(shmid, NULL, 0);
    while (true) {
        if (memcmp(&previousState, &currentSharedData, sizeof(SharedData)) != 0) {

            std::cout << "Token: " << sharedData->token
                << " Bid: " << sharedData->bid
                << " BidQty: " << sharedData->bidqty
                << " AskQty: " << sharedData->askqty
                << " Ask: " << sharedData->ask
                << " LTP: " << sharedData->ltp
                << " LTQ: " << sharedData->ltq
                << " LTT: " << sharedData->ltt
                << " ExchangeTimestamp: " << sharedData->exchange_timestamp << std::endl;
        }
    }

    if (shmdt(sharedData) == -1) {
        perror("shmdt");
        return 1;
    }

    return 0;
}
