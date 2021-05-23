
#pragma once

#include <string>
#include <cstdint>
#include <iostream>

namespace Commands {
    bool getBinaryData(const std::string& filePath, size_t offset, size_t length, std::string& dataOut);
    bool convertRPXToELF(const std::string& rpxPath, const std::string& outPath);
    bool convertELFToRPX(const std::string& elfPath, const std::string& outPath);
    bool yaz0Decompress(std::istream& in, std::ostream& out);
    bool yaz0Compress(std::istream& in, std::ostream& out);
}