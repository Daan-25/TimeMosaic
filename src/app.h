#pragma once
#include "mosaic.h"
#include "color.h"
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <filesystem>
#include <map>

namespace tm {

// Application-wide settings
struct Settings {
    bool calendar_view   = true;   // true = calendar, false = linear
    bool week_start_mon  = true;
    int  cell_size       = 16;     // on-screen cell size
    int  export_cell     = 32;     // export cell size
    int  linear_wrap     = 365;
    bool dominant_color  = false;  // false = average, true = k-means dominant
    bool fast_downsample = true;
};

// Thumbnail cache entry
struct ThumbEntry {
    unsigned int tex_id = 0;
    int w = 0, h = 0;
    bool loaded = false;
};

// Main application state
class App {
public:
    App();
    ~App();

    // Start scanning a folder in background
    void start_scan(const std::string& folder_path);
    // Cancel running scan
    void cancel_scan();
    // Is scanning active?
    bool is_scanning() const { return scanning_.load(); }

    // Access mosaic model (lock before reading in render thread)
    std::mutex& model_mutex() { return model_mutex_; }
    const MosaicModel& model() const { return model_; }

    // Selected day
    CalendarDate selected_day;
    bool has_selection = false;

    // Settings
    Settings settings;

    // Progress
    std::atomic<int> scan_progress{0};
    std::atomic<int> color_progress{0};
    int total_files = 0;

    // Status message
    std::string status_message;

    // Folder path input
    char folder_path[1024] = {};

    // Thumbnail cache
    std::map<std::filesystem::path, ThumbEntry> thumb_cache;

    // Preview
    std::filesystem::path preview_file;
    unsigned int preview_tex = 0;
    int preview_w = 0, preview_h = 0;
    bool show_preview = false;

    // Export status
    std::string export_status;

private:
    void scan_worker(std::string folder);
    MosaicModel model_;
    std::mutex model_mutex_;
    std::atomic<bool> scanning_{false};
    std::atomic<bool> cancel_{false};
    std::thread worker_;
};

} // namespace tm
