# TimeMosaic

A cross-platform C++17 GUI application that builds a calendar-like mosaic from a folder of photos. Each day is represented as a colored cell based on the dominant/average color of photos taken that day. Click a day to preview thumbnails, and export the mosaic as a high-resolution PNG poster.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![CMake](https://img.shields.io/badge/CMake-3.16%2B-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

## Features

- **Folder Ingest**: Drag & drop a folder or enter a path to recursively scan for images
- **Smart Date Detection**: Extracts dates from EXIF metadata, filename patterns (`YYYYMMDD`, `YYYY-MM-DD`, `IMG_20260301`), or file modification time
- **Calendar Heatmap View**: GitHub-contributions-style grid (columns = weeks, rows = days)
- **Linear Mosaic View**: Wrapped row layout for poster-like look
- **Day Inspector**: Click any day to see thumbnails and preview photos
- **PNG Export**: Export calendar or linear mosaic at configurable cell sizes
- **Background Processing**: Scans and computes colors in a worker thread; UI stays responsive
- **Color Modes**: Average color (fast) or dominant color via k-means clustering

## Supported Image Formats

- `.jpg` / `.jpeg` (with EXIF date extraction)
- `.png`
- `.bmp`
- `.tga`

## Building

### Prerequisites

- **CMake** 3.16 or later
- **C++17** compiler (GCC 8+, Clang 7+, MSVC 2017+)
- **OpenGL** development libraries
- On Linux: `sudo apt install libgl-dev libx11-dev libwayland-dev libxkbcommon-dev xorg-dev`
- On macOS: Xcode Command Line Tools
- On Windows: Visual Studio 2017+ with C++ workload

### Build Steps

```bash
# Configure
cmake -S . -B build

# Build
cmake --build build

# Run
./build/TimeMosaic        # Linux/macOS
build\TimeMosaic.exe      # Windows
```

## Usage

1. **Load a folder**: Enter a folder path in the top bar and click "Load", or drag & drop a folder onto the window
2. **Wait for processing**: A progress bar shows scanning and color computation progress
3. **Explore the mosaic**: Hover over cells to see date, photo count, and color info
4. **Inspect a day**: Click a cell to see thumbnails in the right panel
5. **Preview photos**: Click a thumbnail to open a larger preview
6. **Export**: Click "Export Calendar" or "Export Linear" to save a PNG

### Settings

| Setting | Description |
|---------|-------------|
| View | Calendar heatmap or linear mosaic |
| Week Start | Monday or Sunday |
| Cell Size | On-screen cell size (4–64 px) |
| Export Cell | Export cell size (8–128 px) |
| Linear Wrap | Cells per row in linear view |
| Color Mode | Average (fast) or dominant (k-means) |
| Fast Downsample | Stride-based sampling for speed |

## Project Structure

```
CMakeLists.txt          # Build configuration (FetchContent for GLFW + ImGui)
src/
  main.cpp              # GLFW + ImGui bootstrap, drag-and-drop
  app.h / app.cpp       # App state, background worker thread
  scan.h / scan.cpp     # Recursive filesystem scanning
  exif.h / exif.cpp     # EXIF DateTimeOriginal via TinyEXIF
  time_parse.h/cpp      # Filename date patterns + fallback
  image_io.h / cpp      # stb_image wrappers, thumbnail generation
  color.h / color.cpp   # Average color + k-means dominant color
  mosaic.h / mosaic.cpp # Day model, calendar grid mapping
  export_png.h/cpp      # PNG export via stb_image_write
  ui.h / ui.cpp         # All ImGui rendering
  stb_impl.cpp          # stb_image/stb_image_write implementation
third_party/
  stb/                  # stb_image.h, stb_image_write.h
  TinyEXIF/             # TinyEXIF header-only EXIF parser
```

## Tech Stack

- **[Dear ImGui](https://github.com/ocornut/imgui)** – Immediate-mode GUI
- **[GLFW](https://www.glfw.org/)** – Window/input/OpenGL context
- **[stb_image](https://github.com/nothings/stb)** – Image decoding
- **[stb_image_write](https://github.com/nothings/stb)** – PNG export
- **[TinyEXIF](https://github.com/cdcseacave/TinyEXIF)** – JPEG EXIF metadata

## License

MIT License – see [LICENSE](LICENSE) for details.