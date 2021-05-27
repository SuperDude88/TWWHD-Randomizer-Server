
#include "../filetypes/yaz0.hpp"

#include <fstream>

int main(int argc, char** argv)
{
    std::string outFilename{};
    if (argc < 3)
    {
        std::cout << "Usage: yaz0test [-e/-d] [in] [out]" << std::endl;
        return 1;
    }

    std::string flag(argv[1]);
    std::string filename(argv[2]);

    if (argc >= 4)
    {
        outFilename = std::string(argv[3]);
    }
    else
    {
        outFilename = filename;
    }

    std::cout << flag << " " << filename << std::endl;
    // MUST BE OPEN IN BINARY MODE YA IDIOT 
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open())
    {
        std::cout << "Unable to open file " << filename << std::endl;
        return 1;
    }

    if (flag.compare("-e") == 0)
    {
        if (argc < 4) outFilename += ".yaz0";
        std::ofstream out(outFilename, std::ios::binary);
        if(FileTypes::yaz0Encode(in, out) == 0)
        {
            std::cout << "Unable to encode given file" << std::endl;
            return 1;
        }
    }
    else if(flag.compare("-d") == 0)
    {
        if (argc < 4) outFilename += ".dec";
        std::ofstream out(outFilename, std::ios::binary);
        std::cout << "decoding..." << std::endl;
        if(FileTypes::yaz0Decode(in, out) == 0)
        {
            std::cout << "Unable to decode given file" << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "must be one of -e or -d" << std::endl;
        return 1;
    }

    return 0;
}
