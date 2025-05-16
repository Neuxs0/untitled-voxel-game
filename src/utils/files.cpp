#include "files.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace Utils {

std::string readFileToString(const std::string &path, bool &success) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR:Utils:readFileToString: Failed to open file: " << path << std::endl;
        success = false;
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (file.bad()) {
        std::cerr << "ERROR:Utils:readFileToString: Failed to read file: " << path << std::endl;
        success = false;
        return "";
    }

    file.close();
    success = true;
    return content;
}

} // namespace Utils
