#include "ui.h"
#include "app.h"
#include "mosaic.h"
#include "image_io.h"
#include "export_png.h"
#include "imgui.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <GL/gl.h>
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

namespace tm {

// Helper: upload RGBA pixels to an OpenGL texture
static unsigned int upload_texture(const uint8_t* data, int w, int h) {
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    return tex;
}

static void draw_mosaic_calendar(App& app) {
    std::lock_guard<std::mutex> lk(app.model_mutex());
    auto& model = app.model();
    if (model.days.empty()) {
        ImGui::Text("No data loaded. Drop a folder or enter a path above.");
        return;
    }

    int cell = app.settings.cell_size;
    auto min_d = model.min_date;
    auto max_d = model.max_date;

    auto first_dow = day_of_week(min_d);
    if (!app.settings.week_start_mon) first_dow = (first_dow + 1) % 7;

    int num_days = 0;
    { auto d = min_d; while (d <= max_d) { num_days++; d = next_day(d); } }

    int num_cols = (num_days + first_dow + 6) / 7;
    int num_rows = 7;

    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(static_cast<float>(num_cols * cell), static_cast<float>(num_rows * cell));
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Background
    dl->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                      IM_COL32(30, 30, 30, 255));

    auto d = min_d;
    int day_idx = 0;
    while (d <= max_d) {
        int linear = day_idx + first_dow;
        int col = linear / 7;
        int row = linear % 7;

        float x = canvas_pos.x + col * cell;
        float y = canvas_pos.y + row * cell;

        Color3 c{40, 40, 40};
        int file_count = 0;
        auto it = model.days.find(d);
        if (it != model.days.end()) {
            c = it->second.color;
            file_count = static_cast<int>(it->second.files.size());
        }

        ImU32 col32 = IM_COL32(c.r, c.g, c.b, 255);
        dl->AddRectFilled(ImVec2(x + 1, y + 1), ImVec2(x + cell - 1, y + cell - 1), col32);

        // Hover/click
        ImGui::SetCursorScreenPos(ImVec2(x, y));
        char id_buf[64];
        std::snprintf(id_buf, sizeof(id_buf), "##day_%04d%02d%02d", d.year, d.month, d.day);
        if (ImGui::InvisibleButton(id_buf, ImVec2(static_cast<float>(cell), static_cast<float>(cell)))) {
            app.selected_day = d;
            app.has_selection = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%04d-%02d-%02d", d.year, d.month, d.day);
            ImGui::Text("Photos: %d", file_count);
            ImGui::Text("Color: R=%d G=%d B=%d", c.r, c.g, c.b);
            ImGui::EndTooltip();
        }

        d = next_day(d);
        day_idx++;
    }

    ImGui::Dummy(canvas_size);
}

static void draw_mosaic_linear(App& app) {
    std::lock_guard<std::mutex> lk(app.model_mutex());
    auto& model = app.model();
    if (model.days.empty()) {
        ImGui::Text("No data loaded.");
        return;
    }

    int cell = app.settings.cell_size;
    int wrap = app.settings.linear_wrap;
    auto min_d = model.min_date;
    auto max_d = model.max_date;

    int num_days = 0;
    { auto d = min_d; while (d <= max_d) { num_days++; d = next_day(d); } }

    int cols = std::min(wrap, num_days);
    int rows = (num_days + cols - 1) / cols;

    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(static_cast<float>(cols * cell), static_cast<float>(rows * cell));
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                      IM_COL32(30, 30, 30, 255));

    auto d = min_d;
    int idx = 0;
    while (d <= max_d) {
        int c_col = idx % cols;
        int c_row = idx / cols;

        float x = canvas_pos.x + c_col * cell;
        float y = canvas_pos.y + c_row * cell;

        Color3 c{40, 40, 40};
        int file_count = 0;
        auto it = model.days.find(d);
        if (it != model.days.end()) {
            c = it->second.color;
            file_count = static_cast<int>(it->second.files.size());
        }

        ImU32 col32 = IM_COL32(c.r, c.g, c.b, 255);
        dl->AddRectFilled(ImVec2(x + 1, y + 1), ImVec2(x + cell - 1, y + cell - 1), col32);

        ImGui::SetCursorScreenPos(ImVec2(x, y));
        char id_buf[64];
        std::snprintf(id_buf, sizeof(id_buf), "##ld_%04d%02d%02d", d.year, d.month, d.day);
        if (ImGui::InvisibleButton(id_buf, ImVec2(static_cast<float>(cell), static_cast<float>(cell)))) {
            app.selected_day = d;
            app.has_selection = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%04d-%02d-%02d", d.year, d.month, d.day);
            ImGui::Text("Photos: %d", file_count);
            ImGui::EndTooltip();
        }

        d = next_day(d);
        idx++;
    }

    ImGui::Dummy(canvas_size);
}

static void draw_inspector(App& app) {
    if (!app.has_selection) {
        ImGui::Text("Click a day in the mosaic to inspect.");
        return;
    }

    std::lock_guard<std::mutex> lk(app.model_mutex());
    auto& model = app.model();
    auto it = model.days.find(app.selected_day);

    ImGui::Text("Date: %04d-%02d-%02d", app.selected_day.year, app.selected_day.month, app.selected_day.day);

    if (it == model.days.end()) {
        ImGui::Text("No photos on this day.");
        return;
    }

    auto& dd = it->second;
    ImGui::Text("Photos: %d", static_cast<int>(dd.files.size()));
    ImGui::ColorButton("##daycolor",
        ImVec4(dd.color.r / 255.0f, dd.color.g / 255.0f, dd.color.b / 255.0f, 1.0f),
        0, ImVec2(40, 40));
    ImGui::SameLine();
    ImGui::Text("R=%d G=%d B=%d", dd.color.r, dd.color.g, dd.color.b);

    ImGui::Separator();
    ImGui::Text("Thumbnails:");

    float panel_w = ImGui::GetContentRegionAvail().x;
    int thumb_size = 80;
    int cols = std::max(1, static_cast<int>(panel_w / (thumb_size + 8)));

    int col_idx = 0;
    for (auto& f : dd.files) {
        auto& te = app.thumb_cache[f];
        if (!te.loaded) {
            int tw, th;
            auto pixels = load_thumbnail(f, thumb_size, tw, th);
            if (!pixels.empty()) {
                te.tex_id = upload_texture(pixels.data(), tw, th);
                te.w = tw;
                te.h = th;
            }
            te.loaded = true;
        }
        if (te.tex_id) {
            if (col_idx > 0) ImGui::SameLine();
            ImGui::BeginGroup();
            if (ImGui::ImageButton(
                    (std::string("##thumb_") + f.string()).c_str(),
                    reinterpret_cast<ImTextureID>(static_cast<intptr_t>(te.tex_id)),
                    ImVec2(static_cast<float>(te.w), static_cast<float>(te.h)))) {
                app.preview_file = f;
                app.show_preview = true;
                // Load full preview
                if (app.preview_tex) {
                    glDeleteTextures(1, &app.preview_tex);
                    app.preview_tex = 0;
                }
                int pw, ph;
                auto pdata = load_image_rgba(f, pw, ph);
                if (!pdata.empty()) {
                    // Scale down for display if needed
                    app.preview_tex = upload_texture(pdata.data(), pw, ph);
                    app.preview_w = pw;
                    app.preview_h = ph;
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(f.filename().string().c_str());
                ImGui::EndTooltip();
            }
            ImGui::EndGroup();
            col_idx++;
            if (col_idx >= cols) col_idx = 0;
        }
    }
}

static void draw_preview(App& app) {
    if (!app.show_preview || !app.preview_tex) return;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Preview", &app.show_preview)) {
        ImGui::TextUnformatted(app.preview_file.filename().string().c_str());
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float scale = std::min(avail.x / app.preview_w, avail.y / app.preview_h);
        scale = std::min(scale, 1.0f);
        ImGui::Image(
            reinterpret_cast<ImTextureID>(static_cast<intptr_t>(app.preview_tex)),
            ImVec2(app.preview_w * scale, app.preview_h * scale));
    }
    ImGui::End();
}

void render_ui(App& app) {
    // Full-screen dockspace-like layout via a main window
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("TimeMosaic", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // ── Top bar ────────────────────────────────────────────────
    ImGui::Text("Folder:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(400);
    ImGui::InputText("##folder", app.folder_path, sizeof(app.folder_path));
    ImGui::SameLine();
    if (!app.is_scanning()) {
        if (ImGui::Button("Load")) {
            app.start_scan(app.folder_path);
        }
    } else {
        if (ImGui::Button("Cancel")) {
            app.cancel_scan();
        }
    }
    ImGui::SameLine();

    // Export buttons
    {
        std::lock_guard<std::mutex> lk(app.model_mutex());
        bool has_data = !app.model().days.empty();
        if (!has_data) ImGui::BeginDisabled();
        if (ImGui::Button("Export Calendar")) {
            int w, h;
            auto buf = export_calendar_png(app.model(), app.settings.export_cell,
                                           app.settings.week_start_mon, w, h);
            if (!buf.empty() && write_png("timemosaic_calendar.png", buf.data(), w, h)) {
                app.export_status = "Exported timemosaic_calendar.png (" + std::to_string(w) + "x" + std::to_string(h) + ")";
            } else {
                app.export_status = "Export failed.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Export Linear")) {
            int w, h;
            auto buf = export_linear_png(app.model(), app.settings.export_cell,
                                         app.settings.linear_wrap, w, h);
            if (!buf.empty() && write_png("timemosaic_linear.png", buf.data(), w, h)) {
                app.export_status = "Exported timemosaic_linear.png (" + std::to_string(w) + "x" + std::to_string(h) + ")";
            } else {
                app.export_status = "Export failed.";
            }
        }
        if (!has_data) ImGui::EndDisabled();
    }

    // Status
    ImGui::SameLine();
    ImGui::TextUnformatted(app.status_message.c_str());
    if (!app.export_status.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", app.export_status.c_str());
    }

    // Progress bar
    if (app.is_scanning()) {
        if (app.total_files > 0) {
            float frac = static_cast<float>(app.color_progress.load()) / app.total_files;
            ImGui::ProgressBar(frac, ImVec2(-1, 0), "Processing...");
        } else {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Scanning... %d files", app.scan_progress.load());
            ImGui::ProgressBar(-1.0f * static_cast<float>(ImGui::GetTime()), ImVec2(-1, 0), buf);
        }
    }

    ImGui::Separator();

    // ── Three columns: Settings | Mosaic | Inspector ───────────
    float total_w = ImGui::GetContentRegionAvail().x;
    float settings_w = 200.0f;
    float inspector_w = 280.0f;
    float mosaic_w = total_w - settings_w - inspector_w - 20.0f;
    if (mosaic_w < 200.0f) mosaic_w = 200.0f;

    // Settings panel
    ImGui::BeginChild("Settings", ImVec2(settings_w, 0), ImGuiChildFlags_Borders);
    ImGui::Text("Settings");
    ImGui::Separator();

    const char* view_items[] = {"Calendar", "Linear"};
    int view_idx = app.settings.calendar_view ? 0 : 1;
    if (ImGui::Combo("View", &view_idx, view_items, 2)) {
        app.settings.calendar_view = (view_idx == 0);
    }

    const char* week_items[] = {"Monday", "Sunday"};
    int week_idx = app.settings.week_start_mon ? 0 : 1;
    if (ImGui::Combo("Week Start", &week_idx, week_items, 2)) {
        app.settings.week_start_mon = (week_idx == 0);
    }

    ImGui::SliderInt("Cell Size", &app.settings.cell_size, 4, 64);
    ImGui::SliderInt("Export Cell", &app.settings.export_cell, 8, 128);
    ImGui::SliderInt("Linear Wrap", &app.settings.linear_wrap, 30, 1000);

    const char* color_items[] = {"Average", "Dominant"};
    int color_idx = app.settings.dominant_color ? 1 : 0;
    if (ImGui::Combo("Color Mode", &color_idx, color_items, 2)) {
        app.settings.dominant_color = (color_idx == 1);
    }

    ImGui::Checkbox("Fast Downsample", &app.settings.fast_downsample);
    ImGui::EndChild();

    ImGui::SameLine();

    // Mosaic panel
    ImGui::BeginChild("Mosaic", ImVec2(mosaic_w, 0), ImGuiChildFlags_Borders,
        ImGuiWindowFlags_HorizontalScrollbar);
    if (app.settings.calendar_view) {
        draw_mosaic_calendar(app);
    } else {
        draw_mosaic_linear(app);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Inspector panel
    ImGui::BeginChild("Inspector", ImVec2(inspector_w, 0), ImGuiChildFlags_Borders);
    ImGui::Text("Day Inspector");
    ImGui::Separator();
    draw_inspector(app);
    ImGui::EndChild();

    ImGui::End();

    // Preview window
    draw_preview(app);
}

} // namespace tm
