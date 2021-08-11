#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <vector>
#include <string>
#include <fstream>
#include <cassert>

#include "GUIDUtils.h"

namespace JoyEngine {
    static std::vector<char> readFile(const std::string &filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

//        assert(file.is_open());
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        std::streamsize fileSize = file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}

#endif //FILEUTILS_H