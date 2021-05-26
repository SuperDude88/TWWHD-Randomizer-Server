
#include "../filetypes/sarc.hpp"
#include <iostream>
#include <fstream>
#include <vector>

#ifdef _WIN32
constexpr char PATHSEP = '\\';
#else
constexpr char PATHSEP = '/';
#endif

int unpackSARC(const std::string& sarcPath, const std::string& outPath)
{
    FileTypes::SARCFile sarc{};
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

int packSARC(const std::vector<std::string>& filenames, const std::string& outFile)
{
    auto sarc = FileTypes::SARCFile::createNew(outFile);
    SARCError err = SARCError::NONE;

    for (const auto& filename : filenames)
    {
        std::ifstream ifile(filename, std::ios::binary);
        if (!ifile.is_open())
        {
            std::cout << "Unable to open file " << filename << std::endl;
            return 1;
        }
        auto lastDelimPos = filename.find_last_of(PATHSEP);
        std::string storedFilename;
        if (lastDelimPos == std::string::npos)
        {
            storedFilename = filename;
        }
        else
        {
            storedFilename = filename.substr(lastDelimPos + 1);
        }
        if ((err = sarc.addFile(storedFilename, ifile)) != SARCError::NONE)
        {
            std::cout << "Got error adding file: " << FileTypes::SARCErrorGetName(err) << std::endl;
            return 1;
        }
    }

    if ((err = sarc.writeToFile(outFile)) != SARCError::NONE)
    {
        std::cout << "Unable to write out SARC: " << FileTypes::SARCErrorGetName(err) << std::endl;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    
    //if (argc < 4)
    //{
    //    std::cout << "Usage: sarctest [-p/-u] [outfile] [infile(s)]" << std::endl;
    //    return 1;
    //}
    argc = 5;
    argv[1] = "-p";
    argv[2] = R"~(C:\workspace\wiiu_hacks\temp\SARC\TF_01_Room1.szs.repack)~";
    argv[3] = R"~(C:\workspace\wiiu_hacks\temp\SARC\Room1.bfres)~";
    argv[4] = R"~(C:\workspace\wiiu_hacks\temp\SARC\model.sharcfb)~";
    

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
        return packSARC(toPack, outfile);
    }
    else 
    {
        toUnpack = argv[3];
        return unpackSARC(toUnpack, outfile);
    }
}
