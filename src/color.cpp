#include "color.h"
#include "stb_image.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <numeric>
#include <random>

namespace tm {

Color3 compute_average_color(const std::filesystem::path& file, bool fast) {
    int w, h, ch;
    unsigned char* data = stbi_load(file.string().c_str(), &w, &h, &ch, 3);
    if (!data) return {};
    long long rr = 0, gg = 0, bb = 0;
    int count = 0;
    int stride = fast ? std::max(1, (w * h) / (64 * 64)) : 1;
    for (int i = 0; i < w * h; i += stride) {
        rr += data[i * 3 + 0];
        gg += data[i * 3 + 1];
        bb += data[i * 3 + 2];
        count++;
    }
    stbi_image_free(data);
    if (count == 0) return {};
    return {static_cast<uint8_t>(rr / count),
            static_cast<uint8_t>(gg / count),
            static_cast<uint8_t>(bb / count)};
}

Color3 compute_dominant_color(const std::filesystem::path& file, int k) {
    int w, h, ch;
    unsigned char* data = stbi_load(file.string().c_str(), &w, &h, &ch, 3);
    if (!data) return {};
    // Sample up to ~4096 pixels
    int total = w * h;
    int stride = std::max(1, total / 4096);
    struct Pixel { float r, g, b; };
    std::vector<Pixel> samples;
    samples.reserve(4096);
    for (int i = 0; i < total; i += stride) {
        samples.push_back({static_cast<float>(data[i * 3 + 0]),
                           static_cast<float>(data[i * 3 + 1]),
                           static_cast<float>(data[i * 3 + 2])});
    }
    stbi_image_free(data);
    if (samples.empty()) return {};

    // Initialize centroids with first k samples
    k = std::min(k, static_cast<int>(samples.size()));
    std::vector<Pixel> centroids(k);
    std::mt19937 rng(42);
    for (int i = 0; i < k; i++) {
        centroids[i] = samples[i * samples.size() / k];
    }
    std::vector<int> assign(samples.size(), 0);

    for (int iter = 0; iter < 10; iter++) {
        // Assign
        for (size_t i = 0; i < samples.size(); i++) {
            float best = 1e30f;
            for (int c = 0; c < k; c++) {
                float dr = samples[i].r - centroids[c].r;
                float dg = samples[i].g - centroids[c].g;
                float db = samples[i].b - centroids[c].b;
                float dist = dr * dr + dg * dg + db * db;
                if (dist < best) { best = dist; assign[i] = c; }
            }
        }
        // Update
        std::vector<Pixel> sums(k, {0, 0, 0});
        std::vector<int> counts(k, 0);
        for (size_t i = 0; i < samples.size(); i++) {
            sums[assign[i]].r += samples[i].r;
            sums[assign[i]].g += samples[i].g;
            sums[assign[i]].b += samples[i].b;
            counts[assign[i]]++;
        }
        for (int c = 0; c < k; c++) {
            if (counts[c] > 0) {
                centroids[c] = {sums[c].r / counts[c], sums[c].g / counts[c], sums[c].b / counts[c]};
            }
        }
    }
    // Return largest cluster
    std::vector<int> counts(k, 0);
    for (auto a : assign) counts[a]++;
    int best_c = 0;
    for (int c = 1; c < k; c++) {
        if (counts[c] > counts[best_c]) best_c = c;
    }
    return {static_cast<uint8_t>(std::clamp(centroids[best_c].r, 0.f, 255.f)),
            static_cast<uint8_t>(std::clamp(centroids[best_c].g, 0.f, 255.f)),
            static_cast<uint8_t>(std::clamp(centroids[best_c].b, 0.f, 255.f))};
}

} // namespace tm
