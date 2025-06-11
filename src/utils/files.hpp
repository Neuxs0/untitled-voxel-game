#pragma once

#include <iostream>

// Namespace for utility functions.
namespace Utils
{
    // Reads a file and returns its content as a string.
    std::string readFile(const std::string &path, bool &success);
}
