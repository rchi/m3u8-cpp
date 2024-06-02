#pragma once

#include <m3u8/Parser.h>

#include <fstream>

class FileParser : public M3UParser {
public:
    FileParser() {}
    FileParser(std::string uri, CallbackFunc callback) : FileParser()
    {
        setCallback(callback);
        parseFile(uri);
    }
    int parseFile(std::string uri)
    {
        std::ifstream file(uri);

        if (!file.is_open()) {
            std::cerr << "Failed to open file\n";
            return 1;
        }

        std::string line;
        while (std::getline(file, line)) {
            parse(line);
        }

        if (callback != NULL) {
            callback(M3u8Callback, m3u8());
        }
        return 0;
    }
};
