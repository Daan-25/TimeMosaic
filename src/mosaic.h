#pragma once
#include "time_parse.h"
#include "color.h"
#include <vector>
#include <map>
#include <filesystem>

namespace tm {

// Data for a single day
struct DayData {
    CalendarDate date;
    Color3 color;
    std::vector<std::filesystem::path> files;
};

// The full mosaic model
struct MosaicModel {
    std::map<CalendarDate, DayData> days;
    CalendarDate min_date;
    CalendarDate max_date;

    void clear() { days.clear(); min_date = {}; max_date = {}; }

    void update_range() {
        if (days.empty()) { min_date = max_date = {}; return; }
        min_date = days.begin()->first;
        max_date = days.rbegin()->first;
    }
};

// Compute day-of-week: 0=Mon ... 6=Sun (ISO 8601)
int day_of_week(const CalendarDate& d);

// Compute ISO week number for a date
int week_number(const CalendarDate& d);

// Return the number of days in a month
int days_in_month(int year, int month);

// Advance date by 1 day
CalendarDate next_day(const CalendarDate& d);

} // namespace tm
