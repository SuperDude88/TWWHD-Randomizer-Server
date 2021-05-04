
#include "CommandHandler.hpp"
#include "../utility/platform.hpp"
#include <sstream>
#include <fstream>
#include <string>

bool CommandHandler::getBinaryData(std::vector<json> args, std::string& response)
{
    size_t offset, length;
    char* data;
    std::stringstream ss{""};
    if (args.size() != 3)
    {
        return false;
    }
    std::string path = args[0].get<std::string>();
    ss << std::hex << args[1].get<std::string>();
    ss >> offset;
    ss << std::hex << args[2].get<std::string>();
    ss >> length;

    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    file.seekg(offset);
    if (file.fail() || file.eof())
    {
        return false;
    }

    data = new char[length];
    file.read(data, length);
    if(file.fail())
    {
        return false;
    }
    
    json header{};
    header["type"] = "binary";
    header["byte_count"] = std::to_string(length);
    response.append(header.dump());
    response.push_back('\n');
    response.append(data, length);
    return true;
}

std::string CommandHandler::handleCommand(const std::string& command)
{
    json cmdJson = json::parse(command);
    Command cmd = cmdJson.get<Command>();
    Utility::platformLog("cmd name: %s\n", cmd.name.c_str());
    return "";
}
