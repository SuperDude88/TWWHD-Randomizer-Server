
#include "../filetypes/sarc.hpp"
#include <iostream>
#include <fstream>
#include <vector>

#ifdef _WIN32
constexpr char PATHSEP = '\\';
#else
constexpr char PATHSEP = '/';
#endif

int unpackSARC(std::string sarcPath, std::string outPath)
{
    FileTypes::SARCFile sarc{sarcPath};
    SARCError err;
    if((err = sarc.loadFromFile(sarcPath)) != SARCError::NONE)
    {
        std::cout << "failed to load from file: " << FileTypes::SARCErrorGetName(err) << std::endl;
        return 1;
    }

    auto fileList = sarc.getFileList();
    for(const auto& file : fileList)
    {
        std::cout << file.fileName << std::endl;
    }

    for (const auto& file : fileList)
    {
        std::string data;
        if((err = sarc.readFile(file, data)) != SARCError::NONE)
        {
            std::cout << "failed to read file " << file.fileName << ": " << FileTypes::SARCErrorGetName(err) << std::endl;
            return 1;
        }
        std::string outFile = outPath + PATHSEP + file.fileName;
        std::ofstream out(outFile, std::ios::binary);
        if (!out.is_open())
        {
            std::cout << "unable to open file " << outFile << " for output" << std::endl;
            return 1;
        }
        if(!out.write(data.data(), data.size()))
        {
            std::cout << "unable to write to file " << outFile << std::endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::cout << "Usage: sarctest [-p/-u] [outfile] [infile(s)]" << std::endl;
        return 1;
    }

    std::string flag(argv[1]);
    bool pack = true;
    if (flag.compare("-p") == 0)
    {
        pack = true;
    }
    else if (flag.compare("-u") == 0)
    {
        pack = false;
    }

    // in unpack case, outfile is a directory
    // in pack case, outfile is a destination sarc file
    std::string outfile(argv[2]);
    // if we are packing, we can expect multiple input files
    std::vector<std::string> toPack{};
    std::string toUnpack;
    if(pack)
    {
        for(int argIdx = 3; argIdx < argc; argIdx++)
        {
            toPack.emplace_back(argv[argIdx]);
        }
        std::cout << "Not yet implemented!" << std::endl;
        return 1;
    }
    else 
    {
        toUnpack = argv[3];
        return unpackSARC(toUnpack, outfile);
    }
}
