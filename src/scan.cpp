#include "scan.h"
#include <algorithm>

namespace tm {

ScanResult scan_folder(const std::filesystem::path& root,
                       std::atomic<bool>& cancel,
                       std::atomic<int>& progress) {
    ScanResult result;
    namespace fs = std::filesystem;
    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (cancel.load()) break;
        if (ec) continue;
        if (!it->is_regular_file(ec)) continue;
        if (ec) continue;
        progress.fetch_add(1);
        result.scanned++;
        if (is_supported_image(it->path().extension())) {
            result.files.push_back(it->path());
            result.matched++;
        }
    }
    return result;
}

} // namespace tm
