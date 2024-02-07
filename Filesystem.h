#pragma once

#include <string>
#include <direct.h>
#include <iostream>
#include <fstream>

// File functions
namespace Filesys {


std::string GetCurrentDir() // Returns EXE directory
{
    char cCurrentPath[FILENAME_MAX]; // get working directory into buffer
    if (!_getcwd(cCurrentPath, sizeof(cCurrentPath)))
        exit(-1);
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; // not really required

    char* s = cCurrentPath; // save path from buffer into currentpath chararr
    std::string str(s);
    //free(s);
    return str;
}

bool FileExists(const std::string& name) {
    if (FILE* file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    }
    else {
        return false;
    }
}
// Read text file line by line
std::vector<std::string> ReadFromFile(const std::string& filepath) {
    std::vector<std::string> lines;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        return lines; // Return an empty vector if file cannot be opened
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    file.close();
    return lines;
}

bool WriteToFile(const std::string& filepath, const std::vector<std::string>& lines) {
    std::ofstream file(filepath, std::ofstream::trunc); // Open file for writing (truncate existing content)

    if (!file.is_open()) {
        
        return false;
    }

    for (const auto& line : lines) {
        file << line << std::endl;
    }

    file.close();
    return true;
}

}