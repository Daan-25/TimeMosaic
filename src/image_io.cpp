#include "image_io.h"
#include "stb_image.h"
#include <algorithm>
#include <cstring>

namespace tmosaic {

std::vector<uint8_t> load_thumbnail(const std::filesystem::path& file, int size, int& out_w, int& out_h) {
    int w, h, ch;
    unsigned char* data = stbi_load(file.string().c_str(), &w, &h, &ch, 4);
    if (!data) { out_w = out_h = 0; return {}; }

    // Compute thumbnail dimensions
    float scale = static_cast<float>(size) / static_cast<float>(std::max(w, h));
    if (scale >= 1.0f) {
        // Image is smaller than requested thumbnail
        out_w = w;
        out_h = h;
        std::vector<uint8_t> result(data, data + w * h * 4);
        stbi_image_free(data);
        return result;
    }
    int tw = std::max(1, static_cast<int>(w * scale));
    int th = std::max(1, static_cast<int>(h * scale));
    out_w = tw;
    out_h = th;

    // Simple box-filter downscale
    std::vector<uint8_t> thumb(tw * th * 4);
    for (int ty = 0; ty < th; ty++) {
        for (int tx = 0; tx < tw; tx++) {
            int sx = tx * w / tw;
            int sy = ty * h / th;
            sx = std::min(sx, w - 1);
            sy = std::min(sy, h - 1);
            int si = (sy * w + sx) * 4;
            int di = (ty * tw + tx) * 4;
            thumb[di + 0] = data[si + 0];
            thumb[di + 1] = data[si + 1];
            thumb[di + 2] = data[si + 2];
            thumb[di + 3] = data[si + 3];
        }
    }
    stbi_image_free(data);
    return thumb;
}

std::vector<uint8_t> load_image_rgba(const std::filesystem::path& file, int& out_w, int& out_h) {
    int ch;
    unsigned char* data = stbi_load(file.string().c_str(), &out_w, &out_h, &ch, 4);
    if (!data) { out_w = out_h = 0; return {}; }
    std::vector<uint8_t> result(data, data + out_w * out_h * 4);
    stbi_image_free(data);
    return result;
}

} // namespace tmosaic
