
#pragma once

#ifdef PLATFORM_DKP
#include <string>
#include <vector>
#include <coreinit/mcp.h>
#include <nn/act.h>

// most of this taken from https://github.com/emiyl/dumpling

namespace Utility {

    enum class dumpLocation : uint8_t {
        Unknown,
        SDFat,
        USBFat,
        USBExFAT, // TODO: Add exFAT support
        USBNTFS, // TODO: Add NTFS support
    };

    enum class titleLocation : uint8_t {
        Unknown,
        Nand,
        USB,
        Disc,
    };

    struct titlePart {
        std::string path;
        std::string outputPath;
        uint16_t version;
        MCPAppType type;
        uint32_t partHighID;
        titleLocation location;
    };

    struct userAccount {
        bool currAccount = false;
        bool defaultAccount = false;
        bool networkAccount;
        bool passwordCached;
        std::string miiName;
        nn::act::SlotNo slot;
        nn::act::PersistentId persistentId;
    };

    struct titleSave {
        userAccount* account;
        std::string path;
    };

    struct titleSaveCommon {
        std::string path;
        titleLocation location;
    };

    struct titleEntry {
        uint32_t titleLowID;
        std::string shortTitle = "";
        std::string productCode = "";
        std::string normalizedTitle = "";
        bool hasBase = false;
        titlePart base;
        bool hasUpdate = false;
        titlePart update;
        bool hasDLC = false;
        titlePart dlc;
        std::vector<titleSave> saves;
        titleSaveCommon commonSave;
    };

    bool getRawTitles(std::vector<MCPTitleListType>& rawTitlesOut);
    bool loadDetailedTitles(std::vector<MCPTitleListType>& rawTitles, std::vector<titleEntry>& titlesOut);

    std::string normalizeTitle(std::string& unsafeTitle);
    bool isBase(MCPAppType type);
    bool isUpdate(MCPAppType type);
    bool isDLC(MCPAppType type);
    bool isSystemApp(MCPAppType type);
}

#endif
