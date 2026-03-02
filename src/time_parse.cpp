#include "time_parse.h"
#include "exif.h"
#include <regex>
#include <ctime>
#include <chrono>

namespace tmosaic {

CalendarDate parse_exif_date(const std::string& s) {
    CalendarDate d;
    // "YYYY:MM:DD ..." or "YYYY-MM-DD ..."
    if (s.size() >= 10) {
        char sep1 = s[4], sep2 = s[7];
        if ((sep1 == ':' || sep1 == '-') && (sep2 == ':' || sep2 == '-')) {
            try {
                d.year  = std::stoi(s.substr(0, 4));
                d.month = std::stoi(s.substr(5, 2));
                d.day   = std::stoi(s.substr(8, 2));
            } catch (...) { d = {}; }
        }
    }
    return d;
}

CalendarDate parse_filename_date(const std::string& stem) {
    CalendarDate d;
    // Pattern: YYYYMMDD anywhere (IMG_20260301_xxx)
    std::regex re8("(\\d{4})(\\d{2})(\\d{2})");
    std::smatch m;
    if (std::regex_search(stem, m, re8)) {
        try {
            d.year  = std::stoi(m[1].str());
            d.month = std::stoi(m[2].str());
            d.day   = std::stoi(m[3].str());
            if (d.valid() && d.year >= 1970 && d.year <= 2100) return d;
        } catch (...) {}
        d = {};
    }
    // Pattern: YYYY-MM-DD
    std::regex reDash("(\\d{4})-(\\d{2})-(\\d{2})");
    if (std::regex_search(stem, m, reDash)) {
        try {
            d.year  = std::stoi(m[1].str());
            d.month = std::stoi(m[2].str());
            d.day   = std::stoi(m[3].str());
            if (d.valid() && d.year >= 1970 && d.year <= 2100) return d;
        } catch (...) {}
        d = {};
    }
    return d;
}

CalendarDate date_from_file_time(const std::filesystem::path& file) {
    CalendarDate d;
    std::error_code ec;
    auto ftime = std::filesystem::last_write_time(file, ec);
    if (ec) return d;
    // Convert file_time to system_clock::time_point
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
    std::tm* lt = std::localtime(&tt);
    if (lt) {
        d.year  = lt->tm_year + 1900;
        d.month = lt->tm_mon + 1;
        d.day   = lt->tm_mday;
    }
    return d;
}

CalendarDate best_date_for_file(const std::filesystem::path& file) {
    // 1) EXIF
    std::string exif = exif_date(file);
    if (!exif.empty()) {
        auto d = parse_exif_date(exif);
        if (d.valid()) return d;
    }
    // 2) Filename
    auto d = parse_filename_date(file.stem().string());
    if (d.valid()) return d;
    // 3) File modification time
    return date_from_file_time(file);
}

} // namespace tmosaic
