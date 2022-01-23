//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/menubar.hpp>
#include <nana/gui/widgets/menu.hpp>
#include <nana/gui/widgets/tabbar.hpp>

namespace api
{
    struct WayMeta;
    class PC;
};

// Native (Nana) UI
namespace nui
{
    class MainForm;

    std::string dateToShowValue(size_t date, std::tm& tm);

    struct WayElement : nana::group
    {
        WayElement(MainForm& me);

    private:
        void init();
        void loadWayMetas();

    private:
        MainForm& me_;
        nana::listbox lBox_;
        nana::menu menu_;
        nana::button buttonAnalyze_;
        std::vector<api::WayMeta> lBoxToWays_;
    };

    struct DateElement : nana::group
    {
        DateElement(MainForm& me);

        void loadWayDates(const api::WayMeta& wayMeta);

    private:
        void init();

    private:
        MainForm& me_;
        nana::listbox lBox_;
        nana::menu menu_;

        const api::WayMeta* wayMeta_;
        std::vector<size_t> lBoxToDatesOfWay_;
    };

    struct TravelTimeElement : nana::group
    {
        TravelTimeElement(MainForm& me);

        void clearWayDateTimeTravels();
        void loadWayDateTimeTravelHeader(const api::WayMeta& wayMeta);
        void loadWayDateTimeTravels(const api::WayMeta& wayMeta, size_t date);

    private:
        void init();

    private:
        MainForm& me_;
        nana::listbox lBox_;
        nana::menu menu_;
        const api::WayMeta* wayMeta_;
        std::vector<size_t> lBoxToTimestampsOfDateWay_;
    };

    class MainForm : public nana::form
    {
    public:
        MainForm(api::PC& pc, nana::window wd = 0, const::nana::size& sz = { 1024, 800 }, const nana::appearance& apr = { true, true, false, false, true, true, true });

        virtual ~MainForm() override;

    private:
        void init();

    public:
        api::PC& pc_;
        WayElement wayElement_;
        DateElement dateElement_;
        TravelTimeElement travelTimeElement_;

    private:
        nana::place mainPlace_{ *this };
        nana::menubar menuBar_;
    };
}