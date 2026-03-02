#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>

namespace tm {

struct Color3 {
    uint8_t r = 0, g = 0, b = 0;
};

// Load an image file and compute its average color by downsampling.
// `fast` = true uses a stride to skip pixels; false reads every pixel of a 64×64 resize.
Color3 compute_average_color(const std::filesystem::path& file, bool fast = true);

// Simple k-means dominant color (small K). Returns the dominant cluster center.
Color3 compute_dominant_color(const std::filesystem::path& file, int k = 3);

} // namespace tm
