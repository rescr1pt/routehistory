//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#pragma once

#include <set>
#include "MainForm.h"

namespace nui
{
    class PlotForm;
    typedef std::unordered_map<std::string, api::WayMeta> WaysNameMap;

    class WayCheckedColors
    {
    public:
        WayCheckedColors();

        nana::color_rgba getNextColor();
        void getColorByRGBA(nana::color_rgba clr);

        void putColor(nana::color_rgba clr);

        size_t getTotalSize() const;

    private:
        static std::list<nana::color>& getDefColors();

    private:
        std::set<nana::color_rgba> colors_;
    };

    struct GraphConf
    {
        nana::point PLOT_WORK_SIZE;
        nana::point PLOT_TOTAL_SIZE;
        unsigned TRAVELTIME_TOP_MINUTE = 0;
        unsigned TRAVELTIME_MINUTE_MODF = 0;
        unsigned TIME_HOUR_MODF = 0;
        unsigned TRAVELTIME_PANEL_HOUR_INTERVAL_MODF = 0;
    };

    enum class GraphScreenType : unsigned 
    {
        BIG_TO_180M,
        BIG_TO_150M,
        SHORT_TO_150M,
        COUNT,
    };

    const GraphConf& getGraphConf(GraphScreenType type);

    struct DateToWayElement : nana::group
    {
        DateToWayElement(PlotForm& me);

        void loadDates();

    private:
        void init();
        void registerEvents();

    private:
        PlotForm& me_;
        nana::listbox datesLbox_{ *this };
        std::vector<size_t> datesLboxIndexToDates_;
        std::unordered_map<size_t, WaysNameMap > datesWithWays_;
    };

    struct SelectWayElement : nana::group
    {
        SelectWayElement(PlotForm& me);

        void loadDateWays(size_t date, WaysNameMap& wayNameMap);
        void fillDataWays();
        void changeSide();
        void buildAndDrawWaysPosition();

    private:
        void init();
        void registerEvents();

        void setPlotSizeCaption(bool isPlotSizeBig);
        void switchPlotSize();

    private:
        PlotForm& me_;

        nana::listbox waysLbox_{ *this };
        nana::button plotSizeButton_{ *this };
        nana::button turnAroundButton_{ *this };

        std::vector<api::WayMeta> currentWaysLboxIndexToWay_;
        WayCheckedColors wayCheckedColors_;

        bool eventIgnoreWaysLboxSelect_ = false;

        size_t currentDate_ = 0;
        WaysNameMap* currentWays_;
        bool currentIsDirect_ = true;

        // false for first load
        bool currentPlotSizeBig_ = false;
    };


    struct PlotPanelElement : nana::panel<true>
    {
        PlotPanelElement(PlotForm& me);

        void dropSelectedWays();
        void dropSelectedWay(size_t index);
        void addSelectedWay(size_t index, const api::WayMeta& wayMeta, size_t date, const nana::color& clr, bool isDirect);
        void refleshWindow();
        void initPanel();
        void dropRulerPos();

    private:
        void init();
        void registerEvents();

    private:
        PlotForm& me_;

        nana::size::value_type renderedTextH_ = 0;
        nana::size::value_type renderedTextW_ = 0;
        std::pair<nana::point, nana::point> timestampPanelLinePos_;
        std::list<std::pair<nana::point, std::string>> timestampsPanel_;
        std::pair<nana::point, nana::point> travelTimePanelLinePos_;
        std::list<std::pair<nana::point, std::string>> travelTimesPanel_;
        std::unordered_map<size_t, std::pair<nana::color, std::list<nana::point> > > selectedIndexWaysToPoints_;
        nana::point rulerPos_;
    };

    class PlotForm : public nana::form
    {

    public:
        PlotForm(nana::window fromWindow, api::PC& pc, const GraphConf* graphConf = &getGraphConf(GraphScreenType::SHORT_TO_150M));

        void rediv();

    private:
        void init();

    public:
        api::PC& pc_;
        const GraphConf* graphConf_;

        PlotPanelElement plotPanelElement_{ *this };
        DateToWayElement dateToWayElement_{ *this };
        SelectWayElement selectWayElement_{ *this };

    private:
        nana::place mainPlace_{ *this };
    };
}