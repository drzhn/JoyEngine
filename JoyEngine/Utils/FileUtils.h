#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <vector>
#include <string>
#include <fstream>
#include <Utils/Assert.h>

namespace JoyEngine {
    template<typename T>
    static std::vector<T> readFile(const std::string &filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        ASSERT(file.is_open());

        std::streamsize fileSize = file.tellg() / sizeof(T);
        std::vector<T> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    static void getFileStream(std::ifstream &stream, const std::string &filename) {
        stream.open(filename);
        ASSERT(stream.is_open());
    }
}

#endif //FILE_UTILS_H