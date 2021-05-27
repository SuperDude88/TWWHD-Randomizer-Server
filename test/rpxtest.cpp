
#include "../filetypes/wiiurpx.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{

    std::string flag;
    std::string inputFile;
    std::string outputFile;

    if (argc < 3)
    {
        std::cout << "Usage: rpxtest [-d/-c] [inputfile] [outputfile]" << std::endl;
        return 1;
    }

    flag = argv[1];
    inputFile = argv[2];
    if (flag.compare("-d") == 0)
    {
        outputFile = inputFile + ".elf";
    }
    else if (flag.compare("-c") == 0)
    {
        outputFile = inputFile + ".rpx";
    }
    else
    {
        std::cout << "Flag must be one of -d/-c" << std::endl;
        return 1;
    }

    if (argc > 3)
    {
        outputFile = argv[3];
    }
    
    std::ifstream in(inputFile, std::ios::binary);
    std::ofstream out(outputFile, std::ios::binary);

    int err = 0;
    if (flag.compare("-d") == 0)
    {
        if ((err = FileTypes::rpx_decompress(in, out)) != 0)
        {
            std::cout << "Unable to decompress: " << err << std::endl;
            return err;
        }
    }
    else if (flag.compare("-c") == 0)
    {
        if ((err = FileTypes::rpx_compress(in, out)))
        {
            std::cout << "Unable to compress: " << err << std::endl;
            return err;
        }
    }
    else
    {
        std::cout << "Flag must be one of -d/-c" << std::endl;
        return 1;
    }
    return 0;
}