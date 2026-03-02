#pragma once
#include <string>
#include <filesystem>

namespace tm {

// Try to extract EXIF DateTimeOriginal from a JPEG file.
// Returns "YYYY:MM:DD HH:MM:SS" or empty string on failure.
std::string exif_date(const std::filesystem::path& file);

} // namespace tm
