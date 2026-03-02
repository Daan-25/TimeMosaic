#include "app.h"
#include "scan.h"
#include "time_parse.h"
#include "color.h"
#include <algorithm>

namespace tm {

App::App() = default;

App::~App() {
    cancel_scan();
    if (worker_.joinable()) worker_.join();
}

void App::start_scan(const std::string& folder_path) {
    cancel_scan();
    if (worker_.joinable()) worker_.join();

    cancel_.store(false);
    scanning_.store(true);
    scan_progress.store(0);
    color_progress.store(0);
    total_files = 0;
    status_message = "Scanning...";
    export_status.clear();

    {
        std::lock_guard<std::mutex> lk(model_mutex_);
        model_.clear();
    }

    worker_ = std::thread(&App::scan_worker, this, folder_path);
}

void App::cancel_scan() {
    cancel_.store(true);
    if (worker_.joinable()) worker_.join();
    scanning_.store(false);
}

void App::scan_worker(std::string folder) {
    namespace fs = std::filesystem;

    // Phase 1: Scan files
    status_message = "Scanning folder...";
    ScanResult sr = scan_folder(fs::path(folder), cancel_, scan_progress);

    if (cancel_.load()) {
        scanning_.store(false);
        status_message = "Scan cancelled.";
        return;
    }

    total_files = static_cast<int>(sr.files.size());
    status_message = "Computing dates and colors...";

    // Phase 2: Group by date and compute colors
    // First, group files by date
    std::map<CalendarDate, std::vector<fs::path>> grouped;
    for (auto& f : sr.files) {
        if (cancel_.load()) break;
        CalendarDate d = best_date_for_file(f);
        if (d.valid()) {
            grouped[d].push_back(f);
        }
    }

    if (cancel_.load()) {
        scanning_.store(false);
        status_message = "Scan cancelled.";
        return;
    }

    // Phase 3: Compute per-day color
    MosaicModel new_model;
    int processed = 0;
    for (auto& [date, files] : grouped) {
        if (cancel_.load()) break;

        DayData dd;
        dd.date = date;
        dd.files = files;

        // Average the colors of all photos that day
        long long rr = 0, gg = 0, bb = 0;
        int count = 0;
        for (auto& f : files) {
            if (cancel_.load()) break;
            Color3 c;
            if (settings.dominant_color) {
                c = compute_dominant_color(f);
            } else {
                c = compute_average_color(f, settings.fast_downsample);
            }
            rr += c.r;
            gg += c.g;
            bb += c.b;
            count++;
            processed++;
            color_progress.store(processed);
        }
        if (count > 0) {
            dd.color = {static_cast<uint8_t>(rr / count),
                        static_cast<uint8_t>(gg / count),
                        static_cast<uint8_t>(bb / count)};
        }
        new_model.days[date] = dd;
    }

    if (!cancel_.load()) {
        new_model.update_range();
        std::lock_guard<std::mutex> lk(model_mutex_);
        model_ = std::move(new_model);
        status_message = "Done. " + std::to_string(total_files) + " images, "
                       + std::to_string(model_.days.size()) + " days.";
    } else {
        status_message = "Scan cancelled.";
    }

    scanning_.store(false);
}

} // namespace tm
