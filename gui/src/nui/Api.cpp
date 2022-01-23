//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#include "Api.h"

size_t api::extractDateFromTimestamp(size_t timestamp)
{
    std::string str = std::to_string(timestamp);
    return std::stoull(str.substr(0, 4 + 2 + 2));
}

std::string api::wayDescriptionWayback(const std::string& description)
{
    static const std::string FLOW = " -> ";

    auto fromPos = description.find(FLOW);
    if (fromPos == std::string::npos) {
        return "";
    }

    std::string from = description.substr(0, fromPos);
    std::string to = description.substr(fromPos + FLOW.size());
    return to + FLOW + from;


/*
    auto directionCharsPos = description.find("->");

    if (directionCharsPos != std::string::npos) {
        std::string replaceStr = description;
        replaceStr.replace(replaceStr.find("->"), 2, "<-");
        return replaceStr;
    }
*/

    return "";
}

std::string api::secToTime(size_t secs)
{
    int hr = (int)(secs / 3600);
    int min = ((int)(secs / 60)) % 60;
    int sec = (int)(secs % 60);

    char timestring[9];
    sprintf_s(timestring, sizeof(timestring), "%02d%02d%02d", hr, min, sec);

    return timestring;
}

void api::extractDateFromTimestamp(size_t timestamp, std::tm& tmInfo, size_t& date)
{
    std::string str = std::to_string(timestamp);
    date = std::stoull(str.substr(0, 4 + 2 + 2));

    int year = std::stoul(str.substr(0, 4));
    int mon = std::stoul(str.substr(4, 2));
    int day = std::stoul(str.substr(6, 2));

    tmInfo.tm_year = year - 1900;
    tmInfo.tm_mon = mon - 1;
    tmInfo.tm_mday = day;
}
