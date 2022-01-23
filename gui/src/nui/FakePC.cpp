//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#include "FakePC.h"
#include <ctime>

std::string gen_random(const int len) {

    std::string tmp_s;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        ".,-"
        ;



    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];


    return tmp_s;

}

void genTimestamps(std::list<size_t>& timeList)
{
    char buffer[256];

    for (int year = 2021; year < 2023; ++year) {
        for (int mon = 1; mon < 13; ++mon) {
            for (int day = 1; day < 31; ++day) {
                for (int hour = 0; hour < 24; ++hour) {
                    for (int minute = 0; minute < 60; minute += 15) {
                            tm timeStruct;

                            timeStruct.tm_year = year - 1900;
                            timeStruct.tm_mon = mon - 1;
                            timeStruct.tm_mday = day;
                            timeStruct.tm_hour = hour;
                            timeStruct.tm_min = minute;
                            timeStruct.tm_sec = 23;
                            timeStruct.tm_isdst = 0;

                            strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &timeStruct);
                            timeList.push_back(std::stoull(buffer));
                        }
                }
            }
        }
    }
}

void genDate(std::list<size_t>& timeList)
{
    char buffer[256];

    for (int year = 2021; year < 2023; ++year) {
        for (int mon = 1; mon < 13; ++mon) {
            for (int day = 1; day < 31; ++day) {
                tm timeStruct;
                timeStruct.tm_year = year - 1900;
                timeStruct.tm_mon = mon - 1;
                timeStruct.tm_mday = day;
                timeStruct.tm_isdst = 0;

                strftime(buffer, sizeof(buffer), "%Y%m%d", &timeStruct);
                timeList.push_back(std::stoull(buffer));
            }
        }
    }
}

void getTime(std::list<size_t>& timeList)
{
    for (int hour = 0; hour < 24; ++hour) {
        for (int minute = 0; minute < 60; minute += 15) {
            int curSec = 23;

            size_t secs = api::hmsToSecs(hour, minute, curSec);
            timeList.push_back(secs);
        }
    }
}


void api::FakePC::getWayMetaList(std::list<WayMeta>& wayMetaList) const
{
    for (size_t i = 0; i < 15; ++i) {
        wayMetaList.resize(wayMetaList.size() + 1);
        auto& wayMeta = wayMetaList.back();
        wayMeta.guid = wayMetaList.size();
        wayMeta.name = "idea_" + gen_random(12);
    }
}

void api::FakePC::getWayDateList(const WayMeta& /*wayMeta*/, std::list<size_t>& dateList) const
{
    std::list<size_t> dates;
    genDate(dates);

    for (auto& d : dates) {
        dateList.push_back(d);
    }
}

void api::FakePC::getWayTimeTravelListByDate(const WayMeta& wayMeta, size_t dateFilter, std::list<TimeTravelRow>& timeTravelList) const
{
    std::list<size_t> dateList;

    getWayDateList(wayMeta, dateList);

    for (auto& curDate : dateList) {
        if (curDate != dateFilter) {
            continue;
        }

        std::list<size_t> timeList;
        getTime(timeList);

        for (auto& curTime : timeList) {

            timeTravelList.resize(timeTravelList.size() + 1);
            auto& val = timeTravelList.back();
            val.timeSecs_ = curTime;
            std::string timestampStr = std::to_string(curDate) + api::secToTime(curTime);

            val.timestamp = std::stoull(timestampStr);
            val.forward = "1 hour 24 min";
            val.back = "57 min";
        }
    }

}

void api::FakePC::getWayTimeTravelPath(const WayMeta& /*wayMeta*/, size_t /*timestamp*/, TimeTravelPath& timestampPath) const
{
    timestampPath.direct_ = "../../../src/nui/testdata/20210902230750_d.png";
    timestampPath.reverse_ = "../../../src/nui/testdata/20210902230750_r.png";
}

