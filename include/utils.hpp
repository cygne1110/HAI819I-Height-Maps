#pragma once

#include <iostream>
#include <string>

// HEADER-ONLY
// Collection of utility functions that doesn't fit in other files

inline std::string stripPath(std::string src) {

    int lastSlashIndex = 0;
    int size = src.length();
    for(int i = 0; i < size; i++) {
        if(src[i] == '/') lastSlashIndex = i;
    }

    return src.substr(lastSlashIndex + 1);

}

inline std::string getExtension(std::string src) {

    int dotIndex = 0;
    bool foundDot = false;
    int size = src.length();
    for(int i = 0; i < size; i++) {
        if(src[i] == '.') {
            dotIndex = i;
            if(!foundDot) {
                foundDot = true;
            } else {
                std::cerr << "Path provided for shader " << src << " contains an ambiguous extension\n";
                return "";
            }
        }
    }

    return src.substr(dotIndex + 1);

}