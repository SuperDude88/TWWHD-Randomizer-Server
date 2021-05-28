
#pragma once

#include "WWHDStructs.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <unordered_set>

enum struct RandoSessionError
{
    NONE = 0,
    COULD_NOT_OPEN,
    COUNT
};

class RandoSession
{
public:
    using fspath = std::filesystem::path;
    RandoSession(const fspath& gameBaseDir, 
                 const fspath& randoWorkingDir, 
                 const fspath& outputDir);

    fspath getCachePath(const std::vector<std::string>& gameFilePath);

private:
    enum struct CacheEntryType
    {
        DECOMPRESSED_YAZ0,
        UNPACKED_SARC,
        PLAIN_FILE
    };

    struct FileCacheEntry
    {
        CacheEntryType type;
        // for unpacked SARC, this can be multiple unpacked files
        // for yaz0 its just the uncompressed file
        std::vector<std::string> paths;
    };

    fspath extractFile(const std::vector<std::string>& fileSpec);
    
    fspath relToGameAbsolute(const fspath& relPath);
    fspath absToGameRelative(const fspath& absPath);

    fspath gameBaseDirectory;
    fspath workingDir;
    fspath outputDir;
    std::unordered_set<std::string> fileCache;
};
