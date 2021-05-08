
#include "CommandHandler.hpp"
#include "../utility/platform.hpp"
#include <sstream>
#include <fstream>
#include <string>

constexpr size_t MAX_READ_LENGTH = 1024;

void CommandHandler::makeErrorJson(std::string errorMessage, std::string& response)
{
    json err;
    err["type"] = "error";
    err["message"] = errorMessage;
    response.append(err.dump());
}

bool CommandHandler::getBinaryData(const std::vector<json>& args, std::string& response)
{
    size_t offset = 0, length = 0;
    char* data;
    std::stringstream ss{""};
    if (args.size() != 3)
    {
        makeErrorJson("getBinaryData expects 3 arguments", response);
        return false;
    }
    std::string path = args[0].get<std::string>();
    ss << std::hex << args[1].get<std::string>();
    ss >> offset;
    if (ss.fail())
    {
        makeErrorJson("Improperly formatted offset", response);
        return false;
    }
    ss.str("");
    ss.seekg(0);
    ss << args[2].get<std::string>();
    ss >> length;
    if (ss.fail())
    {
        makeErrorJson("Improperly formatted length", response);
        return false;
    }
    ss.clear();
    ss.seekg(0);

    if (length > MAX_READ_LENGTH)
    {
        ss.str("");
        ss << "Can only read up to " << MAX_READ_LENGTH << " bytes.";
        makeErrorJson(ss.str(), response);
        return false;
    }

    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        makeErrorJson("Unable to open file", response);
        return false;
    }
    file.seekg(offset);
    if (!file.good())
    {
        makeErrorJson("file is shorter than start position", response);
        return false;
    }

    data = new char[length];
    file.read(data, length);
    if(file.fail())
    {
        delete[] data;
        makeErrorJson("reached end of file while reading", response);
        return false;
    }
    
    json header{};
    header["type"] = "binary";
    header["byte_count"] = std::to_string(length);
    response.append(header.dump());
    response.push_back('\n');
    response.append(data, length);
    delete[] data;
    return true;
}

bool CommandHandler::handleCommand(const std::string& command, std::string& response)
{
    // handles parse fail case
    json cmdJson;
    cmdJson = json::parse(command, nullptr, false);
    if(cmdJson.is_discarded())
    {
        Utility::platformLog("Unable to parse as json:\n%s\n", command.c_str());
        makeErrorJson("Unable to parse as json", response);
        return false;
    }
    Command cmd = cmdJson.get<Command>();
    Utility::platformLog("cmd name: %s\n", cmd.name.c_str());
    // if we end up with a lot of commands, create some kind of name -> enum -> switch mapping

    if (cmd.name.compare("getBinaryData") == 0)
    {
        return getBinaryData(cmd.args, response);
    }

    makeErrorJson("Command Name " + cmd.name + " is unknown", response);
    return false;
}
