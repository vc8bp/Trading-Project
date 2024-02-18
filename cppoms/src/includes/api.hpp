#pragma once

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

typedef http::response<http::dynamic_body> resT;

resT fetch(const char* endpoint, const char* method, bool isSsl);
