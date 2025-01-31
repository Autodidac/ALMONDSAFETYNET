#pragma once

//#include "alsExports_DLL.h"

//#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace almond {

    class ImageLoader {
    public:
        // ImageData
        struct ImageData {
            int width = 0;       // Initialize width to 0
            int height = 0;      // Initialize height to 0
            int channels = 0;    // Initialize channels to 0 (for example, RGB or RGBA)
            std::vector<uint8_t> pixels; // Pixel data in row-major order (empty by default)
        };

        // Loads an image from file and returns its data
        static ImageData LoadAlmondImage(const std::filesystem::path& filepath);

    private:
        // Helper methods for specific formats
        static ImageData LoadBMP(const std::filesystem::path& filepath);
        static ImageData LoadPNG(const std::filesystem::path& filepath);

        // Add support for other formats as needed
    };

} // namespace almond
