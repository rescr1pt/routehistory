//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#pragma once

#include "Api.h"

namespace api
{
    class GuiPC : public PC
    {
    public:
        GuiPC(const std::string& workingPath);
        virtual ~GuiPC() {}

        virtual void getWayMetaList(std::list<WayMeta>& wayMetaList) const;
        virtual void getWayDateList(const WayMeta& wayMeta, std::list<size_t>& timestampList) const;
        virtual void getWayTimeTravelListByDate(const WayMeta& wayMeta, size_t dateFilter, std::list<TimeTravelRow>& timeTravelList) const;
        virtual void getWayTimeTravelPath(const WayMeta& wayMeta, size_t timestamp, TimeTravelPath& timestampPath) const;

    private:
        std::string workingPath_;

    };
}