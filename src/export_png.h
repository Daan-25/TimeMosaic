#pragma once
#include "mosaic.h"
#include <vector>
#include <cstdint>
#include <string>

namespace tm {

// Render the calendar-heatmap mosaic to a pixel buffer (RGBA).
// Returns pixel data; sets out_w, out_h.
// `cell` = cell size in pixels, `week_start_mon` = true → Monday first.
std::vector<uint8_t> export_calendar_png(const MosaicModel& model,
                                         int cell,
                                         bool week_start_mon,
                                         int& out_w, int& out_h);

// Render the linear mosaic to a pixel buffer (RGBA).
// `wrap` = pixels per row.
std::vector<uint8_t> export_linear_png(const MosaicModel& model,
                                       int cell,
                                       int wrap,
                                       int& out_w, int& out_h);

// Write RGBA buffer to PNG file. Returns true on success.
bool write_png(const std::string& path, const uint8_t* data, int w, int h);

} // namespace tm
