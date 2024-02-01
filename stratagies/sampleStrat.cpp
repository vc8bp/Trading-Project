#include <iostream>

// int client(const char* ip_address, const char* port, const char* token);
int getFeed(int token);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <token>" << std::endl;
        return 1;
    }


    int token = atoi(argv[1]);

    getFeed(token);
    return 0;
}