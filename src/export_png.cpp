#include "export_png.h"
#include "stb_image_write.h"
#include <cstring>

namespace tm {

static void fill_cell(std::vector<uint8_t>& buf, int img_w, int cx, int cy, int cell, Color3 col) {
    for (int dy = 0; dy < cell; dy++) {
        for (int dx = 0; dx < cell; dx++) {
            int px = cx + dx;
            int py = cy + dy;
            int idx = (py * img_w + px) * 4;
            buf[idx + 0] = col.r;
            buf[idx + 1] = col.g;
            buf[idx + 2] = col.b;
            buf[idx + 3] = 255;
        }
    }
}

std::vector<uint8_t> export_calendar_png(const MosaicModel& model,
                                         int cell,
                                         bool week_start_mon,
                                         int& out_w, int& out_h) {
    if (model.days.empty()) { out_w = out_h = 0; return {}; }

    auto min_d = model.min_date;
    auto max_d = model.max_date;

    // Count total weeks
    int total_weeks = 0;
    {
        auto d = min_d;
        int cur_week = -1;
        while (d <= max_d) {
            int dow = day_of_week(d);
            if (!week_start_mon) dow = (dow + 1) % 7; // shift so Sun=0
            if (dow == 0) total_weeks++;
            d = next_day(d);
        }
        total_weeks++; // last partial week
    }

    // Better approach: assign columns by week offset
    auto first_dow = day_of_week(min_d);
    if (!week_start_mon) first_dow = (first_dow + 1) % 7;

    // Count days
    int num_days = 0;
    { auto d = min_d; while (d <= max_d) { num_days++; d = next_day(d); } }

    int num_cols = (num_days + first_dow + 6) / 7;
    int num_rows = 7;

    out_w = num_cols * cell;
    out_h = num_rows * cell;
    if (out_w <= 0 || out_h <= 0) return {};

    std::vector<uint8_t> buf(out_w * out_h * 4, 0);
    // Fill background with dark gray
    for (int i = 0; i < out_w * out_h; i++) {
        buf[i * 4 + 0] = 30;
        buf[i * 4 + 1] = 30;
        buf[i * 4 + 2] = 30;
        buf[i * 4 + 3] = 255;
    }

    // Place each day
    auto d = min_d;
    int day_idx = 0;
    while (d <= max_d) {
        int linear = day_idx + first_dow;
        int col = linear / 7;
        int row = linear % 7;

        Color3 c{40, 40, 40}; // no-photo color
        auto it = model.days.find(d);
        if (it != model.days.end()) {
            c = it->second.color;
        }
        fill_cell(buf, out_w, col * cell, row * cell, cell, c);

        d = next_day(d);
        day_idx++;
    }
    return buf;
}

std::vector<uint8_t> export_linear_png(const MosaicModel& model,
                                       int cell,
                                       int wrap,
                                       int& out_w, int& out_h) {
    if (model.days.empty()) { out_w = out_h = 0; return {}; }
    auto min_d = model.min_date;
    auto max_d = model.max_date;

    int num_days = 0;
    { auto d = min_d; while (d <= max_d) { num_days++; d = next_day(d); } }

    int cols = std::min(wrap, num_days);
    int rows = (num_days + cols - 1) / cols;

    out_w = cols * cell;
    out_h = rows * cell;
    if (out_w <= 0 || out_h <= 0) return {};

    std::vector<uint8_t> buf(out_w * out_h * 4, 0);
    for (int i = 0; i < out_w * out_h; i++) {
        buf[i * 4 + 0] = 30;
        buf[i * 4 + 1] = 30;
        buf[i * 4 + 2] = 30;
        buf[i * 4 + 3] = 255;
    }

    auto d = min_d;
    int idx = 0;
    while (d <= max_d) {
        int col = idx % cols;
        int row = idx / cols;

        Color3 c{40, 40, 40};
        auto it = model.days.find(d);
        if (it != model.days.end()) {
            c = it->second.color;
        }
        fill_cell(buf, out_w, col * cell, row * cell, cell, c);

        d = next_day(d);
        idx++;
    }
    return buf;
}

bool write_png(const std::string& path, const uint8_t* data, int w, int h) {
    return stbi_write_png(path.c_str(), w, h, 4, data, w * 4) != 0;
}

} // namespace tm
