#include <iostream>

int client(const char* ip_address, const char* port, const char* token);

int main() {
    client("127.0.0.1", "8080", "2");
    return 0;
}