#pragma once
#include <vector>
#include <string>
#include <unordered_map>

enum struct MSBTError
{
	NONE = 0,
	COULD_NOT_OPEN,
	NOT_MSBT,
	UNKNOWN_VERSION,
	UNEXPECTED_VALUE,
	UNKNOWN_SECTION,
	NOT_LBL1,
	NOT_ATR1,
	NOT_TSY1,
	NOT_TXT2,
	REACHED_EOF,
	HEADER_DATA_NOT_LOADED,
	FILE_DATA_NOT_LOADED,
	MSBT_NOT_EMPTY,
	MSBT_IS_EMTPY,
	UNKNOWN
};

struct MSBTHeader {
	char magicMsgStdBn[8];
	uint16_t byteOrderMarker;
	uint16_t unknown_0x00;
	uint16_t version_0x0103;
	uint16_t sectionCount;
	uint16_t unknown2_0x00;
	uint32_t fileSize;
	uint8_t padding_0x00[10];
};

struct Label {
	uint32_t checksum;
	uint8_t length;
	std::string string;
	uint32_t messageIndex;
};

struct LBLEntry {
	uint32_t stringCount;
	uint32_t stringOffset;
	std::vector<Label> labels;
};

struct LBL1Header {
	int offset;
	char magicLBL1[4];
	uint32_t tableSize;
	uint8_t padding_0x00[8];
	uint32_t entryCount;
	std::vector<LBLEntry> entries;
};

struct Attributes {
	uint8_t unknown1;
	uint8_t boxStyle;
	uint8_t drawType;
	uint8_t screenPos;
	uint8_t unknown2_8[7];
	uint8_t initialCamera;
	uint8_t unknown9_10[2];
	uint8_t initialAnim;
	uint8_t unknown11_18[8];
};

struct ATR1Header {
	int offset;
	char magicATR1[4];
	uint32_t tableSize;
	uint8_t padding_0x00[8];
	uint32_t entryCount;
	uint32_t entrySize;
	std::vector<Attributes> entries;
};

struct TSY1Entry {
	uint32_t unknown;
};

struct TSY1Header {
	int offset;
	char magicTSY1[4];
	uint32_t tableSize;
	uint8_t padding_0x00[8];
	std::vector<TSY1Entry> entries;
};

struct TXT2Entry {
	uint32_t offset;
	uint32_t nextOffset;
	std::string message;
};

struct TXT2Header {
	int offset;
	char magicTXT2[4];
	uint32_t tableSize;
	uint8_t padding_0x00[8];
	uint32_t entryCount;
	std::vector <TXT2Entry> entries;
};

struct Message {
	Label label;
	Attributes attributes;
	TSY1Entry TSYEntry;
	TXT2Entry text;
};

class MSBTFile {
public:
	MSBTHeader header;
	LBL1Header LBL1;
	ATR1Header ATR1;
	TSY1Header TSY1;
	TXT2Header TXT2;
	std::unordered_map<std::string, Message> messages_by_label;

	MSBTError readSection(std::istream& msbt);
	MSBTError readFile(std::istream& msbt);
	Message& addMessage(std::string label, Attributes attributes, TSY1Entry TSY, std::string message);
	MSBTError saveFile(std::ostream& out);
};
