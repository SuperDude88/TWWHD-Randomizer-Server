#include "msbt.hpp"
#include "../utility/byteswap.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <iterator>

uint32_t LabelChecksum(uint32_t groupCount, std::string label) {
    unsigned int group = 0;

    for (unsigned int i = 0; i < label.length(); i++) {
        group = group * 0x492;
        group = group + label[i];
        group = group & 0xFFFFFFFF;
    }

    return group % groupCount;
}

MSBTError readHeader(std::istream& msbt, MSBTHeader& header) {
    if (!msbt.read(header.magicMsgStdBn, 8)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicMsgStdBn, "MsgStdBn", 8) != 0)
    {
        return MSBTError::NOT_MSBT;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.unknown_0x00), sizeof(header.unknown_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.version_0x0103), sizeof(header.version_0x0103)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.sectionCount), sizeof(header.sectionCount)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.unknown2_0x00), sizeof(header.unknown2_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.fileSize), sizeof(header.fileSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.byteOrderMarker);
    Utility::byteswap_inplace(header.unknown_0x00);
    Utility::byteswap_inplace(header.version_0x0103);
    Utility::byteswap_inplace(header.sectionCount);
    Utility::byteswap_inplace(header.unknown2_0x00);
    Utility::byteswap_inplace(header.fileSize);

    if (header.unknown_0x00 != 0x0000) return MSBTError::UNEXPECTED_VALUE;
    if (header.version_0x0103 != 0x0103) return MSBTError::UNKNOWN_VERSION;
    if (header.sectionCount != 0x0004) return MSBTError::UNEXPECTED_VALUE;
    if (header.unknown2_0x00 != 0x0000) return MSBTError::UNEXPECTED_VALUE;

    return MSBTError::NONE;
}

MSBTError readLBL1(std::istream& msbt, LBL1Header& header) {
    msbt.seekg(-4, std::ios::cur);
    if (!msbt.read(header.magicLBL1, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicLBL1, "LBL1", 4) != 0)
    {
        return MSBTError::NOT_LBL1;
    }
    header.offset = (int)msbt.tellg() - 4;
    if (!msbt.read(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);

    for (uint32_t i = 0; i < header.entryCount; i++) {
        msbt.seekg(header.offset + 0x14 + i * 0x8, std::ios::beg);
        LBLEntry entry;
        if (!msbt.read(reinterpret_cast<char*>(&entry.stringCount), sizeof(entry.stringCount)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&entry.stringOffset), sizeof(entry.stringOffset)))
        {
            return MSBTError::REACHED_EOF;
        }
        Utility::byteswap_inplace(entry.stringCount);
        Utility::byteswap_inplace(entry.stringOffset);
        msbt.seekg(entry.stringOffset + header.offset + 0x10); //Seek to the start of the entries before the loop so it doesnt reset to the same string each time
        for (uint32_t x = 0; x < entry.stringCount; x++) {
            Label label;
            label.checksum = i;
            if (!msbt.read(reinterpret_cast<char*>(&label.length), sizeof(label.length)))
            {
                return MSBTError::REACHED_EOF;
            }
            label.string.resize(label.length); //Length is 1 bit so no byteswap
            if (!msbt.read(&label.string[0], label.length))
            {
                return MSBTError::REACHED_EOF;
            }
            if (!msbt.read(reinterpret_cast<char*>(&label.messageIndex), sizeof(label.messageIndex)))
            {
                return MSBTError::REACHED_EOF;
            }
            Utility::byteswap_inplace(label.messageIndex);
            entry.labels.push_back(label);
        }
        header.entries.push_back(entry);
    }

    if (msbt.tellg() % 16 != 0) {
        int padding_size = 16 - (msbt.tellg() % 16);
        std::string padding;
        padding.resize(padding_size);
        if (!msbt.read(&padding[0], padding_size)) return MSBTError::REACHED_EOF;
        for (const char& character : padding) {
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }

    return MSBTError::NONE;

}

MSBTError readATR1(std::istream& msbt, ATR1Header& header) {
    msbt.seekg(-4, std::ios::cur);
    if (!msbt.read(header.magicATR1, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicATR1, "ATR1", 4) != 0)
    {
        return MSBTError::NOT_ATR1;
    }
    header.offset = (int)msbt.tellg() - 4;
    if (!msbt.read(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.entrySize), sizeof(header.entrySize)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);
    Utility::byteswap_inplace(header.entrySize);

    for (uint32_t i = 0; i < header.entryCount; i++) {
        msbt.seekg(header.offset + 0x18 + i * header.entrySize, std::ios::beg);
        Attributes attributes;
        if (!msbt.read(reinterpret_cast<char*>(&attributes), sizeof(Attributes)))
        {
            return MSBTError::REACHED_EOF;
        }
        header.entries.push_back(attributes);
    }

    msbt.seekg(header.offset + 0x10 + header.tableSize, std::ios::beg);
    if (msbt.tellg() % 16 != 0) {
        int padding_size = 16 - (msbt.tellg() % 16);
        std::string padding;
        padding.resize(padding_size);
        if (!msbt.read(&padding[0], padding_size)) return MSBTError::REACHED_EOF;
        for (const char& character : padding) {
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }

    return MSBTError::NONE;

}

MSBTError readTSY1(std::istream& msbt, TSY1Header& header) {
    msbt.seekg(-4, std::ios::cur);
    if (!msbt.read(header.magicTSY1, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicTSY1, "TSY1", 4) != 0)
    {
        return MSBTError::NOT_ATR1;
    }
    header.offset = (int)msbt.tellg() - 4;
    if (!msbt.read(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.tableSize);

    for (uint32_t i = header.offset + 0x10; i < (header.tableSize + 0x10 + header.offset); i = i + 4) {
        msbt.seekg(i, std::ios::beg);
        TSY1Entry entry;
        if (!msbt.read(reinterpret_cast<char*>(&entry.unknown), sizeof(entry.unknown)))
        {
            return MSBTError::REACHED_EOF;
        }

        Utility::byteswap_inplace(entry.unknown);

        header.entries.push_back(entry);
    }

    if (msbt.tellg() % 16 != 0) {
        int padding_size = 16 - (msbt.tellg() % 16);
        std::string padding;
        padding.resize(padding_size);
        if (!msbt.read(&padding[0], padding_size)) return MSBTError::REACHED_EOF;
        for (const char& character : padding) {
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }

    return MSBTError::NONE;

}

MSBTError readTXT2(std::istream& msbt, TXT2Header& header) {
    msbt.seekg(-4, std::ios::cur);
    if (!msbt.read(header.magicTXT2, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicTXT2, "TXT2", 4) != 0)
    {
        return MSBTError::NOT_ATR1;
    }
    header.offset = (int)msbt.tellg() - 4;
    if (!msbt.read(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);

    for (uint32_t i = 0; i < header.entryCount; i++) {
        msbt.seekg(header.offset + 0x14 + i * 0x4); //0x14 comes from the header info size
        TXT2Entry entry;
        if (!msbt.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&entry.nextOffset), sizeof(entry.nextOffset)))
        {
            return MSBTError::REACHED_EOF;
        }

        Utility::byteswap_inplace(entry.offset);
        Utility::byteswap_inplace(entry.nextOffset);

        msbt.seekg(header.offset + 0x10 + entry.offset); //Offsets are relative to the "end" of the header, 4 bytes before the offset data starts (add 0x10 instead of 0x14)
        int length;
        if (i + 1 != header.entryCount) { //Check if the index is the last in the file
            length = entry.nextOffset - entry.offset;
        }
        else {
            length = header.tableSize - entry.offset;
            entry.nextOffset = header.tableSize;
        }
        entry.message.resize(length);
        if (!msbt.read(&entry.message[0], length))
        {
            return MSBTError::REACHED_EOF;
        }

        header.entries.push_back(entry);
    }

    if (msbt.tellg() % 16 != 0) {
        int padding_size = 16 - (msbt.tellg() % 16);
        std::string padding;
        padding.resize(padding_size);
        if (!msbt.read(&padding[0], padding_size)) return MSBTError::REACHED_EOF;
        for (const char& character : padding) {
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }

    return MSBTError::NONE;

}

MSBTError writeLBL1(std::ostream& out, LBL1Header& header) {
    header.offset = out.tellp();
    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);
    
    out.write(header.magicLBL1, 4);
    out.write(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize));
    out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));
    out.write(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount));

    int i = 0;
    for (LBLEntry& entry : header.entries) {
        out.seekp(header.offset + 0x14 + 0x8 * i, std::ios::beg);
        Utility::byteswap_inplace(entry.stringCount); //byteswap inplace here and swap back later so all the swaps can be grouped in 1 spot instead of in each write (allows for easier Wii U conversion later)
        Utility::byteswap_inplace(entry.stringOffset);

        out.write(reinterpret_cast<char*>(&entry.stringCount), sizeof(entry.stringCount));
        out.write(reinterpret_cast<char*>(&entry.stringOffset), sizeof(entry.stringOffset));
        std::sort(entry.labels.begin(), entry.labels.end(), [](const Label& a, const Label& b) {
            int IDa = std::stoi(a.string), IDb = std::stoi(b.string);
            return IDa < IDb;
        });
        Utility::byteswap_inplace(entry.stringOffset);
        out.seekp(header.offset + 0x10 + entry.stringOffset, std::ios::beg);
        for (Label& label : entry.labels) {
            Utility::byteswap_inplace(label.messageIndex);
        
            label.length = label.string.size();
            out.write(reinterpret_cast<char*>(&label.length), sizeof(label.length));
            out.write(&label.string[0], label.length);
            out.write(reinterpret_cast<char*>(&label.messageIndex), 4);
        }
        i = i + 1;
    }

    if (out.tellp() % 16 != 0) { //Write padding if needed
        int padding_size = 16 - (out.tellp() % 16);
        std::string padding;
        padding.resize(padding_size, '\xab');
        out.write(&padding[0], padding_size);
    }

    return MSBTError::NONE;

}

MSBTError writeATR1(std::ostream& out, ATR1Header& header) {
    header.offset = out.tellp();
    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);
    Utility::byteswap_inplace(header.entrySize);

    out.write(header.magicATR1, 4);
    out.write(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize));
    out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));
    out.write(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount));
    out.write(reinterpret_cast<char*>(&header.entrySize), sizeof(header.entrySize));

    for (Attributes& attributes : header.entries) {
        out.write(reinterpret_cast<char*>(&attributes), sizeof(Attributes));
    }

    if (out.tellp() % 16 != 0) { //Write padding if needed
        int padding_size = 16 - (out.tellp() % 16);
        std::string padding;
        padding.resize(padding_size, '\xab');
        out.write(&padding[0], padding_size);
    }

    return MSBTError::NONE;

}

MSBTError writeTSY1(std::ostream& out, TSY1Header& header) {
    header.offset = out.tellp();
    Utility::byteswap_inplace(header.tableSize);

    out.write(header.magicTSY1, 4);
    out.write(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize));
    out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));

    for (TSY1Entry& entry : header.entries) {
        Utility::byteswap_inplace(entry.unknown);

        out.write(reinterpret_cast<char*>(&entry.unknown), sizeof(entry.unknown));
    }

    if (out.tellp() % 16 != 0) { //Write padding if needed
        int padding_size = 16 - (out.tellp() % 16);
        std::string padding;
        padding.resize(padding_size, '\xab');
        out.write(&padding[0], padding_size);
    }

    return MSBTError::NONE;

}

MSBTError writeTXT2(std::ostream& out, TXT2Header& header) {
    header.offset = out.tellp();
    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);

    out.write(header.magicTXT2, 4);
    out.write(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize));
    out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));
    out.write(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount));

    for (TXT2Entry& entry : header.entries) {
        Utility::byteswap_inplace(entry.offset);
        
        out.write(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
    }

    for (TXT2Entry& entry : header.entries) { //Instead of looping through stuff with indexes and seeking back and forth, we loop through all the header and offset table data and then write the strings
        out.write(&entry.message[0], entry.message.size());
    }

    if (out.tellp() % 16 != 0) { //Write padding if needed
        int padding_size = 16 - (out.tellp() % 16);
        std::string padding;
        padding.resize(padding_size, '\xab');
        out.write(&padding[0], padding_size);
    }

    return MSBTError::NONE;

}

MSBTError MSBTFile::readSection(std::istream& msbt) {
    char magic[4];
    MSBTError error = MSBTError::NONE;
    if (!msbt.read(magic, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(magic, "LBL1", 4) == 0) {
        error = readLBL1(msbt, LBL1);
        if (error != MSBTError::NONE) return error;
    }
    else if (std::strncmp(magic, "ATR1", 4) == 0) {
        error = readATR1(msbt, ATR1);
        if (error != MSBTError::NONE) return error;
    }
    else if (std::strncmp(magic, "TSY1", 4) == 0) {
        error = readTSY1(msbt, TSY1);
        if (error != MSBTError::NONE) return error;
    }
    else if (std::strncmp(magic, "TXT2", 4) == 0) {
        error = readTXT2(msbt, TXT2);
        if (error != MSBTError::NONE) return error;
    }

    return MSBTError::NONE;

}

MSBTError MSBTFile::readFile(std::istream& msbt) {
    MSBTError error = MSBTError::NONE;

    error = readHeader(msbt, header);
    if (error != MSBTError::NONE) return error;

    for (uint16_t i = 0; i < header.sectionCount; i++) {
        error = readSection(msbt);
        if (error != MSBTError::NONE) return error;
    }

    for (const LBLEntry& entry : LBL1.entries) {
        for (const Label& label : entry.labels) { //Populate map of messages
            Message msg;
            msg.label = label;
            msg.attributes = ATR1.entries[label.messageIndex];
            msg.TSYEntry = TSY1.entries[label.messageIndex];
            msg.text = TXT2.entries[label.messageIndex];
            messages_by_label[label.string] = msg;
        }
    }

    return MSBTError::NONE;
}

Message& MSBTFile::addMessage(std::string label, Attributes attributes, TSY1Entry TSY, std::string message) {
    Message newMessage;

    Label newLabel;
    newLabel.checksum = LabelChecksum(LBL1.entryCount, label); //Entry count is always 0x65 for the main text blocks
    newLabel.length = label.size();
    newLabel.string = label;
    newLabel.messageIndex = TXT2.entryCount; //Number of entries = index of message 1 past the end of existing list

    TXT2Entry newEntry;
    newEntry.message = message;

    newMessage.label = newLabel;
    newMessage.attributes = attributes;
    newMessage.TSYEntry = TSY;
    newMessage.text = newEntry;
    messages_by_label[newLabel.string] = newMessage;

    return newMessage;
}

MSBTError MSBTFile::saveFile(std::ostream& out) {

    MSBTError error = MSBTError::NONE;

    Utility::byteswap_inplace(header.byteOrderMarker);
    Utility::byteswap_inplace(header.unknown_0x00);
    Utility::byteswap_inplace(header.version_0x0103);
    Utility::byteswap_inplace(header.sectionCount);
    Utility::byteswap_inplace(header.unknown2_0x00);
    Utility::byteswap_inplace(header.fileSize);

    out.write(header.magicMsgStdBn, 8);
    out.write((char*)&header.byteOrderMarker, sizeof(header.byteOrderMarker));
    out.write((char*)&header.unknown_0x00, sizeof(header.unknown_0x00));
    out.write((char*)&header.version_0x0103, sizeof(header.version_0x0103));
    out.write((char*)&header.sectionCount, sizeof(header.sectionCount));
    out.write((char*)&header.unknown2_0x00, sizeof(header.unknown2_0x00));
    out.write((char*)&header.fileSize, sizeof(header.fileSize));
    out.write((char*)&header.padding_0x00, sizeof(header.padding_0x00));

    //Go through and update all the sections based on the messages by ID
    LBLEntry temp; //Fill all the entries with blank entries
    std::fill(LBL1.entries.begin(), LBL1.entries.end(), temp);
    for (LBLEntry& entry : LBL1.entries) { //Clear each entry's list of labels
        entry.labels.clear();
    }
    ATR1.entries.resize(messages_by_label.size()); //Make sure these are full size, the file relies on indexes a bunch so we need to replace the right indexes in the list (labels store indexes, they're different)
    TSY1.entries.resize(messages_by_label.size());
    TXT2.entries.resize(messages_by_label.size());

    for (const std::pair<std::string, Message>& message : messages_by_label) {
        LBL1.entries[message.second.label.checksum].stringCount = LBL1.entries[message.second.label.checksum].stringCount + 1;
        LBL1.entries[message.second.label.checksum].labels.push_back(message.second.label);
        ATR1.entries[message.second.label.messageIndex] = message.second.attributes;
        TSY1.entries[message.second.label.messageIndex] = message.second.TSYEntry;
        TXT2.entries[message.second.label.messageIndex] = message.second.text;
    }

    LBL1.tableSize = LBL1.entryCount * 0x8;
    int nextGroupOffset = LBL1.entryCount * 0x8 + 0x4; //First entry starts after the table so they start here, 0x4 extra for the entry count
    for (LBLEntry& entry : LBL1.entries) {
        entry.stringCount = entry.labels.size();
        entry.stringOffset = nextGroupOffset;
        for (Label& label : entry.labels) {
            nextGroupOffset = nextGroupOffset + label.string.size() + 0x5; //loop through the labels in the group and add their length for the next group offset
            LBL1.tableSize = LBL1.tableSize + label.string.size() + 0x5; //Add entry lengths to the table length
        }
    }

    ATR1.entryCount = ATR1.entries.size();
    ATR1.tableSize = ATR1.entryCount * ATR1.entrySize + 0x8; //Table size includes the 8 bytes for entry count + size

    TSY1.tableSize = TSY1.entries.size() * 0x4;

    TXT2.entryCount = TXT2.entries.size();
    int nextOffset = TXT2.entryCount * 0x4 + 0x4; //first offset = offset for each entry + the number of entries value (counts as part of the offset table)
    for (TXT2Entry& entry : TXT2.entries) {
        entry.offset = nextOffset;
        entry.nextOffset = entry.offset + entry.message.size();
        nextOffset = entry.nextOffset;
    }
    TXT2.tableSize = TXT2.entries.back().nextOffset;

    error = writeLBL1(out, LBL1);
    if (error != MSBTError::NONE) return error;
    error = writeATR1(out, ATR1);
    if (error != MSBTError::NONE) return error;
    error = writeTSY1(out, TSY1);
    if (error != MSBTError::NONE) return error;
    error = writeTXT2(out, TXT2);
    if (error != MSBTError::NONE) return error;

    out.seekp(0, std::ios::end);
    header.fileSize = out.tellp();
    out.seekp(0x12, std::ios::beg);

    Utility::byteswap_inplace(header.fileSize);
    out.write((char*)&header.fileSize, sizeof(header.fileSize)); //Update full file size

    return MSBTError::NONE;

}
