#include "../includes/api.hpp"
#include <iostream>


int main() {
    resT res = fetch("shopapi.onrender.com/api/auth/login", "POST", true);
    std::cout<< "Hello world" << std::endl;
    return 0;
}

