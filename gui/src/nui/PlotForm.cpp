//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#include <cassert>
#include "PlotForm.h"
#include "Api.h"

#include <nana/gui/drawing.hpp>
#include <nana/gui/detail/window_manager.hpp>
#include <nana/gui/detail/drawer.hpp>

namespace nui
{
    static unsigned CONTROLPANEL_SIZE = 340;

    const GraphConf& getGraphConf(GraphScreenType type)
    {
        struct Once
        {
            Once()
            {
                {
                    auto& big180 = graphs_[(unsigned)GraphScreenType::BIG_TO_180M];

                    big180.PLOT_WORK_SIZE = nana::point(1440, 900);
                    big180.PLOT_TOTAL_SIZE = nana::point(big180.PLOT_WORK_SIZE.x + 30, big180.PLOT_WORK_SIZE.y + 20);
                    big180.TRAVELTIME_TOP_MINUTE = 180;
                    big180.TRAVELTIME_MINUTE_MODF = big180.PLOT_WORK_SIZE.y / big180.TRAVELTIME_TOP_MINUTE;
                    big180.TIME_HOUR_MODF = 1;
                    big180.TRAVELTIME_PANEL_HOUR_INTERVAL_MODF = 4;
                }
                {
                    auto& big150 = graphs_[(unsigned)GraphScreenType::BIG_TO_150M];

                    big150.PLOT_WORK_SIZE = nana::point(1440, 900);
                    big150.PLOT_TOTAL_SIZE = nana::point(big150.PLOT_WORK_SIZE.x + 30, big150.PLOT_WORK_SIZE.y + 20);
                    big150.TRAVELTIME_TOP_MINUTE = 150;
                    big150.TRAVELTIME_MINUTE_MODF = big150.PLOT_WORK_SIZE.y / big150.TRAVELTIME_TOP_MINUTE;
                    big150.TIME_HOUR_MODF = 1;
                    big150.TRAVELTIME_PANEL_HOUR_INTERVAL_MODF = 3;
                }
                {
                    auto& big150 = graphs_[(unsigned)GraphScreenType::SHORT_TO_150M];

                    big150.PLOT_WORK_SIZE = nana::point(720, 600);
                    big150.PLOT_TOTAL_SIZE = nana::point(big150.PLOT_WORK_SIZE.x + 30, big150.PLOT_WORK_SIZE.y + 20);
                    big150.TRAVELTIME_TOP_MINUTE = 120;
                    big150.TRAVELTIME_MINUTE_MODF = big150.PLOT_WORK_SIZE.y / big150.TRAVELTIME_TOP_MINUTE;
                    big150.TIME_HOUR_MODF = 2;
                    big150.TRAVELTIME_PANEL_HOUR_INTERVAL_MODF = 3;
                }
            }

            GraphConf graphs_[(unsigned)GraphScreenType::COUNT];
        };

        static Once once;
        return once.graphs_[(unsigned)type];
    }

    struct EventIgnore
    {
        EventIgnore(bool& value) : value_(value)
        {
            value_ = true;
        }

        ~EventIgnore()
        {
            value_ = false;
        }

        bool& value_;
    };


    unsigned travelTimeToMinutes(const std::string& travelTime)
    {
        size_t par1 = travelTime.find(' ');
        size_t par2 = travelTime.find(' ', par1 + 1);

        if (par2 != std::string::npos) {
            size_t endPos = travelTime.find(' ', par2 + 1);
            unsigned h = std::stoul(travelTime.substr(0, par1));
            unsigned m = std::stoul(travelTime.substr(par2 + 1, endPos));
            return (h * 60) + m;
        }
        else {
            unsigned minuteOrTotalHour = std::stoul(travelTime.substr(0, par1));

            char firstChar = travelTime[par1 + 1];

            if (firstChar == -47) {
                minuteOrTotalHour *= 60;
            }

            return minuteOrTotalHour;
        }

    }

    unsigned timestampToMinutes(const size_t timestamp)
    {
        std::string dateStr = std::to_string(timestamp).substr(4 + 2 + 2);
        unsigned hh = std::stoul(dateStr.substr(0, 2));
        unsigned mm = std::stoul(dateStr.substr(2, 2));
        unsigned res = (hh * 60) + mm;
        return res;
    }
}

nui::WayCheckedColors::WayCheckedColors()
{
    for (auto& c : getDefColors()) {
        colors_.insert((nana::color_rgba)c.rgba().value);
    }
}

nana::color_rgba nui::WayCheckedColors::getNextColor()
{
    assert(!colors_.empty());

    nana::color_rgba clr = *colors_.begin();
    colors_.erase(clr);
    return clr;
}

void nui::WayCheckedColors::getColorByRGBA(nana::color_rgba clr)
{    
    auto clrIt = colors_.find(clr);
    assert(clrIt != colors_.end());

    colors_.erase(clr);
}

void nui::WayCheckedColors::putColor(nana::color_rgba clr)
{
    if (colors_.find(clr) != colors_.end()) {
        assert(false);
    }

    colors_.insert(clr);
    assert(colors_.size() <= getTotalSize());
}

size_t nui::WayCheckedColors::getTotalSize() const
{
    return getDefColors().size();
}

std::list<nana::color>& nui::WayCheckedColors::getDefColors()
{
    static std::list<nana::color> defColors =
    {
        nana::colors::red,
        nana::colors::green,
        nana::colors::blue,
        nana::colors::dark_orange,
        nana::colors::purple,
        nana::colors::dark_slate_blue
    };

    return defColors;
}


nui::DateToWayElement::DateToWayElement(PlotForm& me)
    : nana::group(me)
    , me_(me)
{
    init();
}


void nui::DateToWayElement::init()
{
    this->caption("Dates");

    datesLbox_.append_header({ "Dates" }, 110);
    datesLbox_.enable_single(true, true);
    datesLbox_.show_header(false);

    registerEvents();

    (*this)["dates"] << datesLbox_;
    this->div("<dates>");
}

void nui::DateToWayElement::loadDates()
{
    std::list<api::WayMeta> wayMetas;
    me_.pc_.getWayMetaList(wayMetas);

    datesLbox_.auto_draw(false);

    for (auto& w : wayMetas) {
        std::list<size_t> timestamps;
        me_.pc_.getWayDateList(w, timestamps);

        for (auto& ts : timestamps) {
            size_t date = 0;
            std::tm myTm{};
            api::extractDateFromTimestamp(ts, myTm, date);

            if (datesWithWays_.find(date) == datesWithWays_.end()) {
                datesLbox_.at(0).append({ dateToShowValue(date, myTm) });
                datesLboxIndexToDates_.push_back(date);
            }

            auto& ways = datesWithWays_[date];
            ways[w.name] = w;
        }
    }

    datesLbox_.auto_draw(true);
}

void nui::DateToWayElement::registerEvents()
{
    datesLbox_.events().selected([&](const nana::arg_listbox&)
    {
        const auto& selected = datesLbox_.selected();
        const auto* elem = !selected.empty() ? &selected.front() : nullptr;

        if (!elem) {
            return;
        }


        size_t date = datesLboxIndexToDates_[elem->item];
        auto& ways = datesWithWays_[date];

        me_.selectWayElement_.loadDateWays(date, ways);
    });
}

nui::SelectWayElement::SelectWayElement(PlotForm& me)
    : nana::group(me)
    , me_(me)
{
    init();
}

void nui::SelectWayElement::init()
{
    this->caption("Routes");

    waysLbox_.append_header({ "Ways" }, 200);
    waysLbox_.show_header(false);
    waysLbox_.checkable(true);

    turnAroundButton_.caption("Turn around --->");

    setPlotSizeCaption(currentPlotSizeBig_);

    (*this)["wayBox"] << waysLbox_;
    (*this)["plotSize"] << plotSizeButton_;
    (*this)["turnAround"] << turnAroundButton_;

    this->div(
        "< vertical "
        "      <wayBox> "
        "      <weight=4> "
        "      < <> <plotSize weight=140> <> weight=25 > "
        "      <weight=4> "
        "      < <> <turnAround weight=140> <> weight=25 > "
        "      <weight=4> "
        ">");

    registerEvents();
}

void nui::SelectWayElement::registerEvents()
{
    turnAroundButton_.events().click([&](const nana::arg_click&)
    {
        changeSide();
    });

    plotSizeButton_.events().click([&](const nana::arg_click&)
    {
        switchPlotSize();
    });

    waysLbox_.events().checked([&](const nana::arg_listbox& arg)
    {
        if (eventIgnoreWaysLboxSelect_) {
            return;
        }

        auto checkeds = waysLbox_.checked();

        if (checkeds.size() > wayCheckedColors_.getTotalSize()) {
            EventIgnore wayLboxInfore(eventIgnoreWaysLboxSelect_);
            arg.item.check(false);
        }


        auto allElems = waysLbox_.at(0);
        for (int i = 0; i < allElems.size(); ++i) {
            auto el = allElems.at(i);

            static const nana::color DEF_COLOR{};

            if (el.checked()) {
                if (el.fgcolor() == DEF_COLOR) {
                    el.fgcolor(nana::color(wayCheckedColors_.getNextColor()));
                }
            }
            else {
                if (el.fgcolor() != DEF_COLOR) {
                    wayCheckedColors_.putColor((nana::color_rgba)el.fgcolor().rgba().value);
                }

                el.fgcolor(DEF_COLOR);
            }
        }

        buildAndDrawWaysPosition();
    });
}

void nui::SelectWayElement::setPlotSizeCaption(bool isPlotSizeBig)
{
    static const std::string CAPTION[2] =
    {
        "Switch to large size",
        "Switch to small size",
    };

    plotSizeButton_.caption(CAPTION[isPlotSizeBig]);
}

void nui::SelectWayElement::switchPlotSize()
{
    currentPlotSizeBig_ = !currentPlotSizeBig_;

    setPlotSizeCaption(currentPlotSizeBig_);

    me_.graphConf_ = currentPlotSizeBig_ ? &getGraphConf(GraphScreenType::BIG_TO_150M) : &getGraphConf(GraphScreenType::SHORT_TO_150M);

    const auto& grfConf = *me_.graphConf_;
    auto centered = nana::API::make_center(me_.owner(), grfConf.PLOT_TOTAL_SIZE.x + CONTROLPANEL_SIZE, grfConf.PLOT_TOTAL_SIZE.y);

    me_.size(nana::size(centered.width, centered.height));
    me_.move(centered.x, centered.y);

    // Re-init panel
    me_.plotPanelElement_.dropRulerPos();
    me_.plotPanelElement_.initPanel();

    // Relocate selected ways
    me_.plotPanelElement_.dropSelectedWays();
    buildAndDrawWaysPosition();

    me_.rediv();
}

void nui::SelectWayElement::loadDateWays(size_t date, WaysNameMap& wayNameMap)
{
    currentDate_ = date;
    currentWays_ = &wayNameMap;

    me_.plotPanelElement_.dropSelectedWays();

    std::unordered_map<size_t, nana::color_rgba> oldCheckedGuids;

    // Move last checked to new date ways
    if (!waysLbox_.checked().empty()) {
        for (auto& ch : waysLbox_.checked()) {
            size_t guid = currentWaysLboxIndexToWay_[ch.item].guid;
            auto clr = waysLbox_.at(0).at(ch.item).fgcolor();
            oldCheckedGuids[guid] = (nana::color_rgba)clr.rgba().value;
        }
    }

    currentWaysLboxIndexToWay_.clear();
    for (auto& w : *currentWays_) {
        currentWaysLboxIndexToWay_.push_back(w.second);
    }

    // drop colors
    auto waysChecked = waysLbox_.checked();
    for (auto& ch : waysChecked) {
        auto el = waysLbox_.at(0).at(ch.item);
        wayCheckedColors_.putColor((nana::color_rgba)el.fgcolor().rgba().value);
    }

    fillDataWays();

    if (!oldCheckedGuids.empty()) {
        for (auto oldIt = oldCheckedGuids.begin(); oldIt != oldCheckedGuids.end(); ++oldIt) {
            for (size_t newWayBoxIndex = 0; newWayBoxIndex < currentWaysLboxIndexToWay_.size(); ++newWayBoxIndex) {
                if (currentWaysLboxIndexToWay_[newWayBoxIndex].guid == oldIt->first) {
                    wayCheckedColors_.getColorByRGBA(oldIt->second);
                    waysLbox_.at(0).at(newWayBoxIndex).fgcolor(oldIt->second);
                    waysLbox_.at(0).at(newWayBoxIndex).check(true);
                    break;
                }
            }
        }
        
    }

    me_.plotPanelElement_.refleshWindow();
}

void nui::SelectWayElement::fillDataWays()
{
    waysLbox_.auto_draw(false);
    waysLbox_.clear();

    auto& ways = *currentWays_;

    for (auto& w : ways) {
        auto box = waysLbox_.at(0);

        if (!w.second.description.empty()) {
            if (currentIsDirect_) {
                box.append({ w.second.description });
            }
            else {
                std::string wayBack = api::wayDescriptionWayback(w.second.description);
                box.append({ wayBack });
            }
        }
        else {
            box.append({ w.second.name });
        }
    }

    waysLbox_.auto_draw(true);
}

void nui::SelectWayElement::changeSide()
{
    currentIsDirect_ = !currentIsDirect_;

    auto lboxItems = waysLbox_.at(0);

    waysLbox_.auto_draw(false);

    for (auto& ch : lboxItems) {
        auto& checkedWay = currentWaysLboxIndexToWay_[ch.pos().item];

        auto wayDesc = currentIsDirect_ ? checkedWay.description : api::wayDescriptionWayback(checkedWay.description);

        lboxItems.at(ch.pos().item).text(0, wayDesc);
    }

    waysLbox_.auto_draw(true);

    me_.plotPanelElement_.dropSelectedWays();

    buildAndDrawWaysPosition();
}

void nui::SelectWayElement::buildAndDrawWaysPosition()
{
    const auto& checked = waysLbox_.checked();

    std::set<size_t> checkedIndexes;

    for (auto& ch : checked) {
        checkedIndexes.insert(ch.item);
    }

    for (size_t ind = 0; ind < currentWaysLboxIndexToWay_.size(); ++ind) {
        if (checkedIndexes.find(ind) == checkedIndexes.end()) {
            me_.plotPanelElement_.dropSelectedWay(ind);
            continue;
        }

        const nana::color clr = waysLbox_.at(0).at(ind)->fgcolor();
        auto& way = currentWaysLboxIndexToWay_[ind];

        me_.plotPanelElement_.addSelectedWay(ind, way, currentDate_, clr, currentIsDirect_);
    }

    me_.plotPanelElement_.refleshWindow();
}

nui::PlotPanelElement::PlotPanelElement(PlotForm& me)
    : nana::panel<true>(me)
    , me_(me)
{
    init();
}


void nui::PlotPanelElement::init()
{
    bgcolor(nana::colors::floral_white);

    registerEvents();
}

void nui::PlotPanelElement::registerEvents()
{
    events().mouse_down([&](const nana::arg_mouse& m)
    {
        const auto& grfConf = *me_.graphConf_;

        if (m.left_button) {
            if (m.pos.y >= grfConf.PLOT_WORK_SIZE.y) {
                rulerPos_.y = 0;
                return;
            }

            rulerPos_.x = m.pos.x;
            rulerPos_.y = m.pos.y;
        }
        else {
            rulerPos_.y = 0;
        }

        nana::API::refresh_window(*this);
    });

    nana::drawing { *this }.draw([&](nana::paint::graphics& graph)
    {
        const auto& grfConf = *me_.graphConf_;

        if (!graph.handle()) {
            return;
        }

        // first render
        if (renderedTextW_ == 0) {
            nana::size ss = graph.text_extent_size("T");
            renderedTextW_ = ss.height;
            renderedTextH_ = ss.width;

            assert(renderedTextW_ > 0 && renderedTextH_ > 0);

            initPanel();
            me_.dateToWayElement_.loadDates();
            return;
        }

        for (auto& d : timestampsPanel_) {
            graph.string(d.first, d.second, nana::colors::black);
        }

        graph.line(timestampPanelLinePos_.first, timestampPanelLinePos_.second, nana::colors::gray_border);

        for (auto& d : travelTimesPanel_) {
            graph.string(d.first, d.second, nana::colors::black);
        }
        graph.line(travelTimePanelLinePos_.first, travelTimePanelLinePos_.second, nana::colors::gray_border);

        if (rulerPos_.y != 0) {
            static const auto ruleColor = nana::colors::gray_border;
            graph.line(nana::point(rulerPos_.x, rulerPos_.y), nana::point(grfConf.PLOT_WORK_SIZE.x, rulerPos_.y), ruleColor);

            // at time line
            {
                graph.line(nana::point(rulerPos_.x, rulerPos_.y), nana::point(rulerPos_.x, grfConf.PLOT_WORK_SIZE.y), ruleColor);
            }

            nana::point textPoint;
            textPoint.x = grfConf.PLOT_WORK_SIZE.x / 2;
            textPoint.y = rulerPos_.y;

            const int textOffset = (renderedTextW_ + (renderedTextW_ / 2));

            if (textPoint.y - textOffset < 0) {
                textPoint.y += textOffset;
            }
            else {
                textPoint.y -= textOffset;
            }

            const int travelTimeMin = (grfConf.PLOT_WORK_SIZE.y - rulerPos_.y) / grfConf.TRAVELTIME_MINUTE_MODF;

            std::string currentMinute = std::to_string(travelTimeMin) + " min (at ";

            // at time
            {
                int atMinutes = rulerPos_.x * grfConf.TIME_HOUR_MODF;

                if (atMinutes < 60) {
                    currentMinute += std::to_string(atMinutes) + "m)";
                }
                else {
                    currentMinute += std::to_string(atMinutes / 60) + ".";
                    currentMinute += std::to_string(atMinutes % 60) + "h)";
                }
            }

            // currentMinute += " x=" + std::to_string(rulerPos_.x) + ",y=" + std::to_string(rulerPos_.y);

            graph.string(textPoint, currentMinute, ruleColor);
        }

        for (auto& wayToPoints : selectedIndexWaysToPoints_) {
            const auto& clr = wayToPoints.second.first;
            auto& points = wayToPoints.second.second;
            auto curIt = points.begin();

            {
                static const int wayPointDotSize = 6;
                static const int midSize = (wayPointDotSize / 2);
                static const int roundRadius = 4;

                for (; curIt != points.end(); ++curIt) {
                    auto rec = nana::rectangle(curIt->x - midSize, curIt->y - midSize, wayPointDotSize, wayPointDotSize);
                    graph.round_rectangle(rec, roundRadius, roundRadius, clr, true, clr);

                }
            }

            // Skip line drawing
            if (points.size() < 2) {
                break;
            }
      
            curIt = points.begin();
            graph.line_begin(curIt->x, curIt->y);
            ++curIt;

            for (; curIt != points.end(); ++curIt) {
                graph.line_to(*curIt, clr);
            }
        }
    });
}


void nui::PlotPanelElement::dropSelectedWays()
{
    selectedIndexWaysToPoints_.clear();
}

void nui::PlotPanelElement::dropSelectedWay(size_t index)
{
    selectedIndexWaysToPoints_.erase(index);
}

void nui::PlotPanelElement::addSelectedWay(size_t index, const api::WayMeta& wayMeta, size_t date, const nana::color& clr, bool isDirect)
{
    const auto& grfConf = *me_.graphConf_;

    // Already set
    if (selectedIndexWaysToPoints_.find(index) != selectedIndexWaysToPoints_.end()) {
        return;
    }

    auto& pointIt = selectedIndexWaysToPoints_[index];

    pointIt.first = clr;

    auto& points = pointIt.second;

    std::list<api::TimeTravelRow> timeTravels;
    me_.pc_.getWayTimeTravelListByDate(wayMeta, date, timeTravels);

    for (auto& tt : timeTravels) {
        const std::string& travelTime = isDirect ? tt.forward : tt.back;
        unsigned travelMin = travelTimeToMinutes(travelTime);
        unsigned timestampMin = timestampToMinutes(tt.timestamp);

        points.resize(points.size() + 1);
        auto& point = points.back();

        int xPoint = timestampMin / grfConf.TIME_HOUR_MODF;
        int yPoint = 0;

        if (travelMin > grfConf.TRAVELTIME_TOP_MINUTE) {
            travelMin = 0;
        }
        else {
            yPoint = grfConf.TRAVELTIME_MINUTE_MODF * travelMin;
            yPoint = grfConf.PLOT_WORK_SIZE.y - yPoint;
        }

        point.x = xPoint;
        point.y = yPoint;
    }
}

void nui::PlotPanelElement::refleshWindow()
{
    nana::API::refresh_window(*this);
}

void nui::PlotPanelElement::initPanel()
{
    const auto& grfConf = *me_.graphConf_;

    timestampsPanel_.clear();
    travelTimesPanel_.clear();

    timestampPanelLinePos_ = std::make_pair(nana::point(0, grfConf.PLOT_WORK_SIZE.y), nana::point(grfConf.PLOT_TOTAL_SIZE.x, grfConf.PLOT_WORK_SIZE.y));

    char buffer[64];

    static const unsigned HOUR_STEP = 60;

    for (int h = 0; h < 24; h += grfConf.TIME_HOUR_MODF) {
        for (int m = 0; m < 60; m += HOUR_STEP) {
            std::pair<nana::point, std::string> p;

            tm timeStruct{};

            timeStruct.tm_hour = h;
            timeStruct.tm_min = m;

            int strSize = (int)strftime(buffer, sizeof(buffer), "%H:%M", &timeStruct);

            int fontOffset = strSize * (renderedTextH_ / 2);

            int xPos = (h * 60) + m;
            xPos /= grfConf.TIME_HOUR_MODF;

            xPos -= fontOffset;

            p.first.x = xPos;
            p.first.y = grfConf.PLOT_WORK_SIZE.y;

            // skip  0
            if (h != 0) {
                p.second = buffer;
            }

            timestampsPanel_.push_back(p);
        }
    }

    travelTimePanelLinePos_ = std::make_pair(nana::point(grfConf.PLOT_WORK_SIZE.x, 0), nana::point(grfConf.PLOT_WORK_SIZE.x, grfConf.PLOT_TOTAL_SIZE.y));

    for (unsigned i = 0; i <= grfConf.TRAVELTIME_TOP_MINUTE; i += grfConf.TRAVELTIME_PANEL_HOUR_INTERVAL_MODF) {
        std::pair<nana::point, std::string> p;
        p.first.x = grfConf.PLOT_WORK_SIZE.x + (renderedTextH_ / 2);
        int yPoint = grfConf.TRAVELTIME_MINUTE_MODF * i;
        p.first.y = grfConf.PLOT_WORK_SIZE.y - yPoint;

        // place to center 
        p.first.y -= (renderedTextW_ / 2);

        // skip 0
        if (i != 0) {
            p.second = std::to_string(i);
        }

        travelTimesPanel_.push_back(p);
    }
}

void nui::PlotPanelElement::dropRulerPos()
{
    rulerPos_.y = 0;
}

nui::PlotForm::PlotForm(nana::window fromWindow, api::PC& pc, const GraphConf* graphConf)
    : pc_(pc)
    , graphConf_(graphConf)
    , nana::form(fromWindow, nana::API::make_center(fromWindow, graphConf->PLOT_TOTAL_SIZE.x + CONTROLPANEL_SIZE, graphConf->PLOT_TOTAL_SIZE.y), { true, true, false, true, false, false, false })
{
    init();
}

void nui::PlotForm::rediv()
{
    mainPlace_.div(
        "horizontal "
        "<workGraph weight=" + std::to_string(graphConf_->PLOT_TOTAL_SIZE.x) + "> "
        "<weight=4> "
        "<dateBox weight=125> "
        "<weight=4> "
        "< routes > "
        "");

    mainPlace_.collocate();
}

void nui::PlotForm::init()
{
    caption("route history - plot");

    mainPlace_["workGraph"] << plotPanelElement_;
    mainPlace_["dateBox"] << dateToWayElement_;
    mainPlace_["routes"] << selectWayElement_;

    rediv();
}
