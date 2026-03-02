#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <filesystem>

namespace tmosaic {

struct ScanResult {
    std::vector<std::filesystem::path> files;
    int scanned = 0;
    int matched = 0;
};

// Supported image extensions (lowercase, with dot).
inline bool is_supported_image(const std::filesystem::path& ext) {
    static const char* exts[] = {".jpg", ".jpeg", ".png", ".bmp", ".tga"};
    std::string s = ext.string();
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    for (auto e : exts)
        if (s == e) return true;
    return false;
}

// Recursively scan `root` for supported images.
// Sets `cancel` to true externally to abort.
// `progress` is updated with #files scanned so far.
ScanResult scan_folder(const std::filesystem::path& root,
                       std::atomic<bool>& cancel,
                       std::atomic<int>& progress);

} // namespace tmosaic
