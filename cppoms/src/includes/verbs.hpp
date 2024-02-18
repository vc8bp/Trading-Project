#pragma once

#include <string>
#include <unordered_map>
#include <boost/beast/http.hpp>

namespace http = boost::beast::http; 

static const std::unordered_map<std::string, http::verb> method_map {
    {"GET", http::verb::get},
    {"POST", http::verb::post},
    {"PUT", http::verb::put},
    {"DELETE", http::verb::delete_},
    {"OPTIONS", http::verb::options},
    {"PATCH", http::verb::patch},
};

http::verb convert_method_to_enum(const std::string &method) {
    auto it = method_map.find(method);
    return it != method_map.end() ? it->second : http::verb::get;
};

std::tuple<std::string, std::string> parseEndpoint(const std::string endpoint) {
    size_t index = endpoint.find("/");

    if(index == std::string::npos) 
        return std::make_tuple(endpoint, "/");

    std::string host = endpoint.substr(0, index);
    std::string target = endpoint.substr(index);
    return std::make_tuple(host, target);
        
}