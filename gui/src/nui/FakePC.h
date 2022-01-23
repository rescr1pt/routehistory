//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#pragma once

#include "Api.h"

namespace api 
{
    class FakePC : public PC
    {
    public:
        void getWayMetaList(std::list<WayMeta>& wayMetaList) const override;
        void getWayDateList(const WayMeta& wayMeta, std::list<size_t>& dateList) const override;
        void getWayTimeTravelListByDate(const WayMeta& wayMeta, size_t dateFilter, std::list<TimeTravelRow>& timeTravelList) const override;
        void getWayTimeTravelPath(const WayMeta& wayMeta, size_t timestamp, TimeTravelPath& timestampPath) const override;
    };
}