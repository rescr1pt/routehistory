//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#include "MainForm.h"
#include "PlotForm.h"
#include "Api.h"
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/widgets/scroll.hpp>

#include <iomanip>

namespace nui
{
    class ScreenshotForm : public nana::form
    {
    public:
        ScreenshotForm(nana::window fromWindow, api::PC& pc, const api::WayMeta& wayMeta, size_t timestamp)
            : nana::form(fromWindow, nana::API::make_center(fromWindow, nana::screen().desktop_size().width, nana::screen().desktop_size().height), { true, true, false, true, false, true, false })
            , pc_(pc)
            , wayMeta_(wayMeta)
            , timestamp_(timestamp)
        {
            init();
        }

        bool wasLoad() const { return wasLoad_; }

    private:
        void init()
        {
            caption("route history - screenshot");

#ifndef NANA_LIBPNG 
#error LIBPNG not enabled
#endif

            pc_.getWayTimeTravelPath(wayMeta_, timestamp_, timeTravelPath_);

            if (!timeTravelPath_.direct_.empty()) {
                auto img = nana::paint::image(timeTravelPath_.direct_);
                if (img.empty()) {
                    return;
                }
                picDirect_.load(std::move(img));
            }
            else {
                return;
            }

            if (!timeTravelPath_.reverse_.empty()) {
                auto img = nana::paint::image(timeTravelPath_.reverse_);

                if (img.empty()) {
                    return;
                }
                
                picReverse_.load(img);
            }
            else {
                return;
            }

            wasLoad_ = true;

            picDirect_.stretchable(true);
            picReverse_.stretchable(true);

            picDirect_.align(nana::align::center, nana::align_v::top);
            picReverse_.align(nana::align::center, nana::align_v::top);

            button_.fgcolor(nana::colors::black);
            button_.bgcolor(nana::colors::white_smoke);

            button_.events().click([&](const nana::arg_click&)
            {
                updateState();
                mainPlace_.collocate();
            });

            updateState();

            mainPlace_["button"] << button_;
            mainPlace_["pic_d"] << picDirect_;
            mainPlace_["pic_r"] << picReverse_;

            mainPlace_.collocate();

        }

        void updateState()
        {
            static std::string DIV_DATA[2] =
            {
                "  vertical <weight=10> < <> <button> <> weight=34> <weight=10>  pic_d ",
                "  vertical <weight=10> < <> <button> <> weight=34> <weight=10>  pic_r "
            };

            static std::string CAPTION_DATA_BASE[2] =
            {
                "<--- Show ",
                "Show --->"
            };

            switch (nextSide_)
            {
                case 0:
                {
                    if (timeTravelPath_.direct_.empty()) {
                        return;
                    }
                    break;
                }
                case 1:
                {
                    if (timeTravelPath_.reverse_.empty()) {
                        return;
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }

            std::string waybackDesc = api::wayDescriptionWayback(wayMeta_.description);
            const std::string& wayCap = waybackDesc.empty() ? CAPTION_DATA_BASE[nextSide_] : (nextSide_ == 0 ? waybackDesc : wayMeta_.description);

            button_.caption(wayCap);
            mainPlace_.div(DIV_DATA[nextSide_]);
            nextSide_ = nextSide_ == 1 ? 0 : 1;
        }

        api::PC& pc_;
        const api::WayMeta& wayMeta_;
        size_t timestamp_;
        bool wasLoad_ = false;
        api::TimeTravelPath timeTravelPath_;
        nana::place mainPlace_{ *this };
        nana::picture picDirect_{ *this };
        nana::picture picReverse_{ *this };
        nana::button button_{ *this };
        unsigned short nextSide_ = 0;
    };

    std::string uiRepDate(const std::string& date)
    {
        std::string repDate;
        repDate = date.substr(6, 2);
        repDate += ".";
        repDate += date.substr(4, 2);
        repDate += ".";
        repDate += date.substr(0, 4);

        return std::move(repDate);
    }

    std::string uiRepTime(size_t timestamp)
    {
        std::string timeStr = std::to_string(timestamp).substr(8);
        std::string timeRep;
        timeRep += timeStr.substr(0, 2);
        timeRep += ":";
        timeRep += timeStr.substr(2, 2);
        timeRep += ":";
        timeRep += timeStr.substr(4, 2);

        return std::move(timeRep);
    }

    std::string nui::dateToShowValue(size_t date, std::tm& tm)
    {
        std::mktime(&tm);
        static const std::string DAY[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };

        std::string showValue = uiRepDate(std::to_string(date));
        showValue += " (" + std::to_string(tm.tm_wday == 0 ? 7 : tm.tm_wday) + " " + DAY[tm.tm_wday] + ")";
        return showValue;
    }
}

nui::WayElement::WayElement(MainForm& me) 
    : nana::group(me)
    , me_(me)
    , lBox_(*this)
    , buttonAnalyze_(*this)
{
    init();
}

void nui::WayElement::init()
{
    this->caption("Routes");

    lBox_.append_header({ "Ways" }, 220);
    lBox_.show_header(false);
    lBox_.enable_single(true, true);

    buttonAnalyze_.caption("Show analytics");

    loadWayMetas();

    (*this)["routes"] << lBox_;
    (*this)["analytics"] << buttonAnalyze_;

    this->div("vertical <routes> <weight=4> <<<> <analytics weight=100> <>> weight=30>");

    lBox_.events().selected([&](const nana::arg_listbox&)
    {
        const auto& selected = lBox_.selected();
        const auto* elem = !selected.empty() ? &selected.front() : nullptr;

        if (!elem) {
            return;
        }

        auto& wayMeta = lBoxToWays_[elem->item];
        me_.dateElement_.loadWayDates(wayMeta);
    });

    buttonAnalyze_.events().click([&](const nana::arg_click&)
    {
        PlotForm plotForm(me_, me_.pc_);
        plotForm.show();
        plotForm.modality();
    });

    lBox_.events().mouse_down([&](const nana::arg_mouse& arg)
    {
        if (arg.right_button) {

/*
            auto addHandler = [&](nana::menu::item_proxy& ip) {
                lBox_.at(0).append({ "Asdasdasjdlkasjdlkasjld" });
            };

            auto deleteHandler = [&](nana::menu::item_proxy& ip) {
                lBox_.erase(lBox_.selected());

            };

            const auto& selected = lBox_.selected();

            menu_.clear();

            if (selected.empty()) {
                menu_.append("Add", addHandler);
            }
            else {
                menu_.append("Delete", deleteHandler);
            }

            menu_.popup(arg.window_handle, arg.pos.x, arg.pos.y);
            */
        }
    });
}

void nui::WayElement::loadWayMetas()
{
    std::list<api::WayMeta> ways;
    me_.pc_.getWayMetaList(ways);
    for (auto& w : ways) {
        const std::string& val = !w.description.empty() ? w.description : w.name;
        lBox_.at(0).append({val});
        lBoxToWays_.push_back(w);
    }
}

nui::DateElement::DateElement(MainForm& me)
    : me_(me)
    , nana::group(me)
    , lBox_(*this)
    , wayMeta_(nullptr)
{
    init();
}

void nui::DateElement::init()
{
    lBox_.append_header({ "Dates" });
    lBox_.show_header(false);
    lBox_.enable_single(true, true);

    this->caption("Dates");
    (*this)["dates"] << lBox_;
    this->div("<dates>");

    lBox_.events().selected([&](const nana::arg_listbox& )
    {
        const auto& selected = lBox_.selected();
        const auto* elem = !selected.empty() ? &selected.front() : nullptr;

        if (!elem) {
            return;
        }

        size_t date = lBoxToDatesOfWay_[elem->item];
        me_.travelTimeElement_.loadWayDateTimeTravels(*wayMeta_, date);
    });
}


void nui::DateElement::loadWayDates(const api::WayMeta& wayMeta)
{
    wayMeta_ = &wayMeta;

    me_.travelTimeElement_.clearWayDateTimeTravels();
    me_.travelTimeElement_.loadWayDateTimeTravelHeader(wayMeta);

    std::list<size_t> timestamps;
    me_.pc_.getWayDateList(wayMeta, timestamps);

    lBoxToDatesOfWay_.clear();
    
    lBox_.auto_draw(false);

    lBox_.erase();
    size_t lastDate = 0;

    for (auto& ts : timestamps) {
        size_t date = 0;
        std::tm myTm{};
        api::extractDateFromTimestamp(ts, myTm, date);
        
        if (lastDate == date) {
            continue;
        }

        lastDate = date;

        lBox_.at(0).append({ dateToShowValue(date, myTm) });
        lBoxToDatesOfWay_.push_back(date);
    }

    lBox_.auto_draw(true);
}

nui::TravelTimeElement::TravelTimeElement(MainForm& me)
    : me_(me)
    , nana::group(me)
    , lBox_(*this)
    , wayMeta_(nullptr)
{
    init();
}

void nui::TravelTimeElement::init()
{
    lBox_.enable_single(true, true);

    this->caption("Travel Times");
    (*this)["travelTimes"] << lBox_;
    this->div("<travelTimes>");

    lBox_.events().click([&](const nana::arg_click& )
    {
        const auto& selected = lBox_.selected();
        const auto* elem = !selected.empty() ? &selected.front() : nullptr;

        if (!elem) {
            return;
        }

        size_t timestamp = lBoxToTimestampsOfDateWay_[elem->item];
        ScreenshotForm screenViewer(me_, me_.pc_, *wayMeta_, timestamp);
        if (screenViewer.wasLoad()) {
            screenViewer.show();
            screenViewer.wait_for_this();

        }
        else {
            nana::msgbox notFoundScreen(me_, "Information");
            notFoundScreen << "No screenshots found for this datetime";
            notFoundScreen.show();
        }
    });
/*

    lBox_.events().mouse_down([&](const nana::arg_mouse& arg)
    {
        if (arg.right_button) {

            auto openScreen = [&](nana::menu::item_proxy& arg) {
                const auto& selected = lBox_.selected();
                const auto* elem = !selected.empty() ? &selected.front() : nullptr;
                
                if (!elem) {
                    return;
                }

                auto cat = lBox_.at(0);
            };

            menu_.clear();

            const auto& selected = lBox_.selected();
            const auto* elem = !selected.empty() ? &selected.front() : nullptr;

            if (!elem) {
                return;
            }

            menu_.append("Open screen", openScreen);

            menu_.popup(arg.window_handle, arg.pos.x, arg.pos.y);
        }
    });*/
}


void nui::TravelTimeElement::clearWayDateTimeTravels()
{
    lBoxToTimestampsOfDateWay_.clear();

    lBox_.auto_draw(false);
    lBox_.clear();
    lBox_.auto_draw(true);
}

void nui::TravelTimeElement::loadWayDateTimeTravelHeader(const api::WayMeta& wayMeta)
{
    lBox_.clear_headers();
    lBox_.append_header({ "Time" });

    std::string waybackDesc = api::wayDescriptionWayback(wayMeta.description);

    if (!waybackDesc.empty()) {
        lBox_.append_header({ wayMeta.description }, 180);
        lBox_.append_header({ waybackDesc }, 180);
    }
    else {
        lBox_.append_header({ "Forward" });
        lBox_.append_header({ "Back" });
    }

}

void nui::TravelTimeElement::loadWayDateTimeTravels(const api::WayMeta& wayMeta, size_t date)
{
    wayMeta_ = &wayMeta;

    lBoxToTimestampsOfDateWay_.clear();

    lBox_.auto_draw(false);

    lBox_.clear();

    std::list<api::TimeTravelRow> timestamps;
    me_.pc_.getWayTimeTravelListByDate(wayMeta, date, timestamps);
    for (auto& ts : timestamps) {
        std::string repTime = uiRepTime(ts.timestamp);
        lBox_.at(0).append({ repTime, ts.forward, ts.back });

        lBoxToTimestampsOfDateWay_.push_back(ts.timestamp);
    }

    lBox_.auto_draw(true);
}

nui::MainForm::MainForm(api::PC& pc, nana::window wd, const::nana::size& sz, const nana::appearance& apr)
    : pc_(pc)
    , nana::form(wd, sz, apr)
    , wayElement_(*this)
    , dateElement_(*this)
    , travelTimeElement_(*this)
{
    init();
}

nui::MainForm::~MainForm()
{

}

void nui::MainForm::init()
{
    caption("route history");

    menuBar_.create(*this);
    menuBar_.push_back("Menu");

    mainPlace_.div(
        " margin=4"
        " vertical "
        " < menus weight=28 > "
        " <horizontal "
        "     <wayPlace weight=260> "
        "     | "
        "     <datePlace weight=200> "
        "     |  "
        "     <travelTimePlace> "
        " > "
        " "
    );

    mainPlace_["menus"] << menuBar_;
    mainPlace_["wayPlace"] << wayElement_;
    mainPlace_["datePlace"] << dateElement_;
    mainPlace_["travelTimePlace"] << travelTimeElement_;
    mainPlace_.collocate();
}

