//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#include "GuiApi.h"
#include <fstream>
#include <string>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

struct FolderScaner
{
    FolderScaner(const std::string& path, std::function<void(const std::string& filename) > caller)
    {
        if (!fs::exists(path)) {
            return;
        }

        for (const auto& entry : fs::directory_iterator(path)) {
            caller(entry.path().filename().string());
        }
    }
};

api::GuiPC::GuiPC(const std::string& workingPath)
    : workingPath_(workingPath)
{

}

void api::GuiPC::getWayMetaList(std::list<WayMeta>& wayMetaList) const
{
    std::unordered_map<std::string, std::string> waysDescriptions;
    std::ifstream waysDescriptionFstream(workingPath_ + "/waysdescription.txt");
    if (waysDescriptionFstream.is_open())
    {
        std::string line;
        while (std::getline(waysDescriptionFstream, line))
        {
            if (line.empty()) {
                continue;
            }

            auto eqPos = line.find('=');
            if (eqPos == std::string::npos) {
                continue;
            }

            std::string paramName = line.substr(0, eqPos);
            std::string paramValue = line.substr(eqPos + 1);
            waysDescriptions[paramName] = paramValue;
        }
    }

    std::ifstream waysFstream(workingPath_ + "/ways.conf");
    if (!waysFstream.is_open())
    {
        throw std::runtime_error("Can not open ways.conf");
    }

    std::string line;
    while (std::getline(waysFstream, line))
    {
        if (line.empty()) {
            continue;
        }

        size_t pos = line.find('=');


        if (pos == std::string::npos) {
            continue;
        }

        WayMeta way;
        way.name = line.substr(0, pos);
        way.guid = wayMetaList.size() + 1;

        auto wayDescIt = waysDescriptions.find(way.name);

        if (wayDescIt != waysDescriptions.end()) {
            way.description = wayDescIt->second;
        }

        if (way.name.empty()) {
            throw std::runtime_error("Way is invalid");
        }

        wayMetaList.push_back(way);
    }

    waysFstream.close();
}

void api::GuiPC::getWayDateList(const WayMeta& wayMeta, std::list<size_t>& dateList) const
{
    std::string path = workingPath_ + "/parsed/" + wayMeta.name;

    auto caller = [&](const std::string& filename) -> void {
        // ex: 20210902
        if (filename.size() != 8) {
            return;
        }

        size_t timestamp = std::stoull(filename);
        dateList.push_back(timestamp);
    };

    FolderScaner(path, caller);
}

void api::GuiPC::getWayTimeTravelListByDate(const WayMeta& wayMeta, size_t dateFilter, std::list<TimeTravelRow>& timeTravelList) const
{
    std::string path = workingPath_ + "/parsed/" + wayMeta.name + "/" + std::to_string(dateFilter);

    auto caller = [&](const std::string& filename) -> void {
        const static std::string FORMAT = ".txt";

        if (filename.size() != 6 + FORMAT.size() || strncmp(filename.c_str() + 6, FORMAT.c_str(), FORMAT.size()) != 0) {
            return;
        }

        const std::string fullFile = path + "/" + filename;
        std::ifstream fin(fullFile);
        if (!fin.is_open()) {
            throw std::runtime_error("Can not open file " + fullFile);
        }

        timeTravelList.resize(timeTravelList.size() + 1);
        auto& val = timeTravelList.back();

        std::string timestampStr = std::to_string(dateFilter);
        timestampStr += filename.substr(0, 6);

        val.timestamp = std::stoull(timestampStr);

        std::string line;
        while (std::getline(fin, line)) {
            if (line.empty()) {
                continue;
            }

            auto eqPos = line.find('=');
            if (eqPos == std::string::npos) {
                continue;
            }

            std::string paramName = line.substr(0, eqPos);
            std::string paramValue = line.substr(eqPos + 1);
            if (paramName == "direct") {
                val.forward = paramValue;
            }
            else if (paramName == "reverse") {
                val.back = paramValue;
            }
        }

        fin.close();
    };

    FolderScaner(path, caller);
}

void api::GuiPC::getWayTimeTravelPath(const WayMeta& wayMeta, size_t timestamp, TimeTravelPath& timestampPath) const
{
    // To refactor: Need to pass 2 arguments - date and time
    std::string timstampStr = std::to_string(timestamp);
    char timeStr[9] = { 0 };
    char dateStr[7] = { 0 };
    memcpy(timeStr, timstampStr.c_str(), 8);
    memcpy(dateStr, timstampStr.c_str() + 8, 6);

    std::string path = workingPath_ + "/parsed/" + wayMeta.name + "/";
    path += timeStr;

    auto caller = [&](const std::string& filename) -> void {
        // _X.png
        if (filename.size() != 6 + 6) {
            return;
        }

        if (strncmp(filename.c_str(), dateStr, 6) != 0) {
            return;
        }

        if (strncmp(filename.c_str() + 6, "_d.png", 6) == 0) {
            timestampPath.direct_ = path + "/" + filename;
        }
        else if (strncmp(filename.c_str() + 6, "_r.png", 6) == 0) {
            timestampPath.reverse_ = path + "/" + filename;
        }
    };

    FolderScaner(path, caller);
}
