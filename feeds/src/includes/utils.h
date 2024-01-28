#include <sstream>

template <typename T>
T convertToNumber(const std::string& input) {
    T result;
    std::istringstream stream(input);
    stream >> result;
    return result;
}
