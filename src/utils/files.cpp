#include "files.hpp"

#include <fstream>

namespace Utils {

std::string readFile(const std::string &path, bool &success)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error: Utils: readFile: Failed to open file: " << path << std::endl;
        success = false;
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (file.bad())
    {
        std::cerr << "Error: Utils: readFile: Failed to read file: " << path << std::endl;
        success = false;
        return "";
    }

    file.close();
    success = true;
    return content;
}

} // namespace Utils
