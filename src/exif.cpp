#include "exif.h"
#include "TinyEXIF.h"
#include <fstream>
#include <algorithm>

namespace tmosaic {

std::string exif_date(const std::filesystem::path& file) {
    // Only attempt on JPEG files
    std::string ext = file.extension().string();
    for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (ext != ".jpg" && ext != ".jpeg") return {};

    std::ifstream ifs(file, std::ios::binary);
    if (!ifs) return {};

    TinyEXIF::EXIFInfo info;
    if (info.parseFrom(ifs) != TinyEXIF::PARSE_SUCCESS) return {};

    if (!info.DateTimeOriginal.empty()) return info.DateTimeOriginal;
    if (!info.DateTime.empty()) return info.DateTime;
    return {};
}

} // namespace tmosaic
