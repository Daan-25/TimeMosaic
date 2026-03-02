#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>

namespace tm {

// Load image, generate a thumbnail (max `size` pixels on longest edge).
// Returns RGBA pixels (4 channels). Sets out_w, out_h.
std::vector<uint8_t> load_thumbnail(const std::filesystem::path& file, int size, int& out_w, int& out_h);

// Decode full image to RGBA. Caller must free with stbi_image_free or use returned vector.
std::vector<uint8_t> load_image_rgba(const std::filesystem::path& file, int& out_w, int& out_h);

} // namespace tm
