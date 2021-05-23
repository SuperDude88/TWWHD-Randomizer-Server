#include "Commands.hpp"
#include "../filetypes/wiiurpx.hpp"

namespace Commands {
    bool getBinaryData(const std::string& filePath, size_t offset, size_t length, std::string& dataOut)
    {
        return false;
    }

    bool convertRPXToELF(const std::string& rpxPath, const std::string& outPath)
    {

        return false;
    }

    bool convertELFToRPX(const std::string& elfPath, const std::string& outPath)
    {
        return false;
    }

    bool yaz0Decompress(std::istream& in, std::ostream& out)
    {
        return false;
    }

    bool yaz0Compress(std::istream& in, std::ostream& out)
    {
        return false;
    }
}
