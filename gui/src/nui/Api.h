//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#pragma once

#include <set>
#include <list>
#include <string>

namespace api
{
    struct WayMeta
    {
        std::string name;
        std::string description;
        size_t guid = 0;
    };
    
    struct TimeTravelRow
    {
        size_t timestamp = 0; // To removing
        size_t timeSecs_ = 0;
        std::string forward;
        std::string back;
    };

    struct TimeTravelPath
    {
        std::string direct_;
        std::string reverse_;
    };

    size_t extractDateFromTimestamp(size_t timestamp);
    void extractDateFromTimestamp(size_t timestamp, std::tm& tmInfo, size_t& date);
    std::string wayDescriptionWayback(const std::string& description);
    std::string secToTime(size_t secs);
    inline size_t hmsToSecs(int hour, int minute, int sec) { return (((size_t)hour * 3600) + ((size_t)minute * 60) + sec); }

    /*

    struct ExtractDigitFromNumber
    {
        struct Iface
        {
            friend struct ExtractDigitFromNumber;
            virtual ~Iface() {}

        protected:
            virtual void extDig(unsigned index, unsigned digit) = 0;
        };

        static inline void extract(size_t number, Iface& iface)
        {
            unsigned index = 0;
            return subExtract(number, index, iface);
        }

    private:
        static void subExtract(size_t number, unsigned& index, Iface& iface)
        {
            if (number >= 10) {
                subExtract(number / 10, index, iface);
            }

            unsigned digit = number % 10;
            iface.extDig(index, digit);
            ++index;
        }
    };*/

	class PC
	{
		public:
            PC() {}
            virtual ~PC() {}
            
			virtual void getWayMetaList(std::list<WayMeta>& wayMetaList) const = 0;
			virtual void getWayDateList(const WayMeta& wayMeta, std::list<size_t>& dateList) const = 0;
            virtual void getWayTimeTravelListByDate(const WayMeta& wayMeta, size_t dateFilter, std::list<TimeTravelRow>& timeTravelList) const = 0;
            virtual void getWayTimeTravelPath(const WayMeta& wayMeta, size_t timestamp, TimeTravelPath& timestampPath) const = 0;
	};
};