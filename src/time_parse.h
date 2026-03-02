#pragma once
#include <string>
#include <chrono>
#include <filesystem>

namespace tmosaic {

// Represents a calendar date (year, month, day).
struct CalendarDate {
    int year  = 0;
    int month = 0; // 1-12
    int day   = 0; // 1-31

    bool valid() const { return year > 0 && month >= 1 && month <= 12 && day >= 1 && day <= 31; }

    bool operator==(const CalendarDate& o) const { return year == o.year && month == o.month && day == o.day; }
    bool operator<(const CalendarDate& o) const {
        if (year != o.year) return year < o.year;
        if (month != o.month) return month < o.month;
        return day < o.day;
    }
    bool operator<=(const CalendarDate& o) const { return !(o < *this); }
};

// Parse "YYYY:MM:DD ..." or "YYYY-MM-DD ..." into a CalendarDate.
CalendarDate parse_exif_date(const std::string& s);

// Try to extract a date from the filename (e.g. IMG_20260301, 2026-03-01, 20260301).
CalendarDate parse_filename_date(const std::string& stem);

// Fallback: file last-write-time → CalendarDate (local time).
CalendarDate date_from_file_time(const std::filesystem::path& file);

// Determine the best date for a file:
// 1) EXIF DateTimeOriginal  2) filename pattern  3) last-write-time
CalendarDate best_date_for_file(const std::filesystem::path& file);

} // namespace tmosaic
