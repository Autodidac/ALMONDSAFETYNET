#pragma once

#include "alsEngineConfig.h"
#include "alsImageLoader.h"  // Assuming ImageLoader is defined elsewhere
#include "alsOpenGLTexture.h"
#include "alsTexture.h"

#ifdef ALMOND_USING_OPENGLTEXTURE

namespace almond
{
    class OpenGLTextureAtlas : public almond::Texture {
    public:
        OpenGLTextureAtlas(const std::filesystem::path& filepath = "../../assets/images/default.bmp", Format format = Format::RGBA8, bool generateMipmaps = true, GLuint initialWidth = 16384, GLuint initialHeight = 16384, GLuint maxSize = 32768)
            : filepath(filepath), format(format), generateMipmaps(generateMipmaps), atlasWidth(initialWidth), atlasHeight(initialHeight), maxAtlasSize(maxSize) {
            LoadAtlasTexture(filepath);
        }

        ~OpenGLTextureAtlas() {
            glDeleteTextures(1, &atlasID);
        }

        void AddTextureToAtlas(const std::filesystem::path& filepath) {

        }

        void Bind(unsigned int slot = 0) const override {
            glBindTexture(GL_TEXTURE_2D, atlasID);
        }

        void Unbind() const override {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        int GetWidth() const override { return atlasWidth; }
        int GetHeight() const override { return atlasHeight; }
        unsigned int GetID() const override { return atlasID; }
        std::filesystem::path GetPath() const override { return filepath; }

        void SetFiltering(GLenum minFilter, GLenum magFilter) const override {
            glBindTexture(GL_TEXTURE_2D, atlasID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        OpenGLTexture GetTexture(const std::filesystem::path& texturePath) {
            for (auto& tex : m_atlastextures) {
                if (tex.GetPath() == texturePath) {
                    return tex;
                }
            }
            throw std::runtime_error("Texture not found in the renderer.");
        }

        std::vector<unsigned char> GetData() const override {
            // Check if the texture is valid before proceeding
            if (atlasID == 0) {
                throw std::runtime_error("Texture atlas is not initialized.");
            }

            // Query the dimensions of the texture if needed
            GLint width, height;
            glBindTexture(GL_TEXTURE_2D, atlasID);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
            glBindTexture(GL_TEXTURE_2D, 0);

            // Validate dimensions
            if (width <= 0 || height <= 0) {
                throw std::runtime_error("Invalid texture dimensions.");
            }

            // Allocate memory for the texture data (RGBA format)
            std::vector<unsigned char> data(width * height * 4);

            // Optionally, use a Pixel Buffer Object (PBO) for async reading of texture data
            GLuint pbo = 0;
            glGenBuffers(1, &pbo);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
            glBufferData(GL_PIXEL_PACK_BUFFER, width * height * 4, nullptr, GL_STATIC_READ);

            // Read texture data into PBO
            glBindTexture(GL_TEXTURE_2D, atlasID);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            // Map the PBO to get the texture data asynchronously
            unsigned char* ptr = static_cast<unsigned char*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
            if (ptr) {
                std::copy(ptr, ptr + width * height * 4, data.begin());
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }
            else {
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                glDeleteBuffers(1, &pbo);
                throw std::runtime_error("Failed to map PBO for texture data.");
            }

            // Unbind PBO and clean up
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            glDeleteBuffers(1, &pbo);

            // Return the texture data
            return data;
        }

        std::tuple<int, int, int, int> TryAddTexture(const std::filesystem::path& filepath) {
            // Load the texture image
            auto image = ImageLoader::LoadAlmondImage(filepath);

            if (!image.pixels.data()) {
                throw std::runtime_error("Failed to load image: " + filepath.string());
            }


            // Find the best position for this texture in the atlas
            auto [xOffset, yOffset, textureWidth, textureHeight] = PackTexture(image.width, image.height);

            // If packing fails (e.g., no available space), return (-1, -1, -1, -1)
            if (xOffset == -1 || yOffset == -1) {
                std::cerr << "Failed to find space for texture in atlas: " << filepath << std::endl;
                return { -1, -1, -1, -1 };
            }

            // Add the texture to the map
            textureMap[filepath.string()] = { xOffset, yOffset, textureWidth, textureHeight };

            // add the texture to the stored atlas textures
            m_atlastextures.emplace_back(filepath, almond::Texture::Format::RGBA8, false);

            // Debug information
#ifdef _DEBUG
            std::cout << "Atlas Size: " << atlasWidth << "x" << atlasHeight << "\n";
            std::cout << "Adding Texture: " << filepath << " at (" << xOffset << ", " << yOffset << ")\n";
            std::cout << "Texture Size: " << textureWidth << "x" << textureHeight << "\n";
#endif

            // Upload the texture to the atlas at the calculated position
            glBindTexture(GL_TEXTURE_2D, atlasID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset,
                textureWidth, textureHeight,
                GL_RGBA, GL_UNSIGNED_BYTE,
                image.pixels.data());
            glBindTexture(GL_TEXTURE_2D, 0);

            // Return the (x, y, width, height) position of the texture in the atlas
            return { xOffset, yOffset, textureWidth, textureHeight };
        }

        void UpdateAtlasData(const std::filesystem::path& filepath, int gridx, int gridy, int gridwidth, int gridheight) {
            // Store the texture position and dimensions in the texture map
            textureMap[filepath.string()] = { gridx, gridy, gridwidth, gridheight };

            // Mark the space as occupied in the atlas (using a simple boolean grid to track space)
            for (int i = gridx; i < gridx + gridwidth; ++i) {
                for (int j = gridy; j < gridy + gridheight; ++j) {
                    occupiedSpaces[i][j] = true;
                }
            }

            // Debugging info (optional)
#ifdef _DEBUG
            std::cout << "Updated Atlas Data: " << filepath << " at position (" << gridx << ", " << gridy << ") with size "
                << gridwidth << "x" << gridheight << "\n";
#endif
        }

        std::vector<OpenGLTexture>& GetTextures() {
            // Return the list of textures currently in the atlas
        }

        std::tuple<int, int, int, int> GetAtlasTextureMap(const std::string& texturePath) const {
            auto it = textureMap.find(texturePath);
            if (it != textureMap.end()) return it->second;
            throw std::runtime_error("Texture not found: " + texturePath);
        }

        void GetUVs(const std::string& texturePath, float& uMin, float& uMax, float& vMin, float& vMax) const {
            auto [xOffset, yOffset, width, height] = GetAtlasTextureMap(texturePath);

            uMin = static_cast<float>(xOffset) / atlasWidth;
            uMax = static_cast<float>(xOffset + width) / atlasWidth;
            vMin = static_cast<float>(yOffset) / atlasHeight;
            vMax = static_cast<float>(yOffset + height) / atlasHeight;
        }

    private:
        unsigned int atlasID = 0;
        GLuint atlasWidth = 16384;
        GLuint atlasHeight = 16384;
        GLuint maxAtlasSize = 32768; // Maximum size of the atlas (e.g., 32768 for 32k)

        std::vector<OpenGLTexture> m_atlastextures;
        std::unordered_map<std::string, std::tuple<int, int, int, int>> textureMap;
        Format format = almond::Texture::Format::RGBA8;
        bool generateMipmaps = true;
        ImageLoader::ImageData image;
        const std::filesystem::path filepath = "";

        // Simple texture packing algorithm (MaxRects)
        std::vector<std::vector<bool>> occupiedSpaces = std::vector<std::vector<bool>>(atlasWidth, std::vector<bool>(atlasHeight, false));

        // Simple texture packing algorithm (MaxRects)
        std::tuple<int, int, int, int> PackTexture(GLuint texWidth, GLuint texHeight) {
            if (texWidth > atlasWidth || texHeight > atlasHeight) {
                throw std::runtime_error("Texture is too large for the current atlas size.");
            }

            // Find a space for the new texture
            for (GLuint y = 0; y + texHeight <= atlasHeight; ++y) {
                for (GLuint x = 0; x + texWidth <= atlasWidth; ++x) {
                    // Check if the space is free
                    if (IsFreeSpace(x, y, texWidth, texHeight)) {
                        // Texture fits, add it to the map and return its position
                        return { x, y, texWidth, texHeight };
                    }
                }
            }

            // If no space is available, resize the atlas
            ResizeAtlas();
            return PackTexture(texWidth, texHeight); // Retry packing in the resized atlas
        }

        bool IsFreeSpace(GLuint x, GLuint y, GLuint texWidth, GLuint texHeight) {
            for (GLuint ty = y; ty < y + texHeight; ++ty) {
                for (GLuint tx = x; tx < x + texWidth; ++tx) {
                    if (textureMap.find(std::to_string(tx) + "_" + std::to_string(ty)) != textureMap.end()) {
                        return false; // Space is already occupied
                    }
                }
            }
            return true; // The space is free
        }

        void ResizeAtlas() 
        {
            // Cache the atlas texture data before resizing
            std::vector<unsigned char> cache(atlasWidth * atlasHeight * 4); // Assuming 4 channels (RGBA)
            glBindTexture(GL_TEXTURE_2D, atlasID);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, cache.data());
            glBindTexture(GL_TEXTURE_2D, 0);

            // Double the atlas size, but don't exceed the maximum size
            GLuint newWidth = atlasWidth * 2;
            GLuint newHeight = atlasHeight * 2;

            // Ensure the new size doesn't exceed the maximum size
            if (newWidth > maxAtlasSize) newWidth = maxAtlasSize;
            if (newHeight > maxAtlasSize) newHeight = maxAtlasSize;

            std::cout << "Resizing Atlas: " << atlasWidth << "x" << atlasHeight << " -> " << newWidth << "x" << newHeight << std::endl;

            // Create a new texture with the resized dimensions
            GLuint newAtlasID;
            glGenTextures(1, &newAtlasID);
            glBindTexture(GL_TEXTURE_2D, newAtlasID);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, cache.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Transfer old textures to the new atlas
            for (const auto& [path, texData] : textureMap) {
                auto [x, y, w, h] = texData;
                glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // Here you should transfer the texture data
            }

            glBindTexture(GL_TEXTURE_2D, 0);

            // Delete old atlas texture
            glDeleteTextures(1, &atlasID);
            atlasID = newAtlasID;

            atlasWidth = newWidth;
            atlasHeight = newHeight;
        }

        void LoadAtlasTexture(const std::filesystem::path& filepath) {
            image = ImageLoader::LoadAlmondImage(filepath.string().c_str());

            if (image.pixels.empty()) {
                throw std::runtime_error("Failed to load image: " + filepath.string());
            }

            atlasWidth = image.width;
            atlasHeight = image.height;

            glGenTextures(1, &atlasID);
            glBindTexture(GL_TEXTURE_2D, atlasID);

            GLenum internalFormat = (image.channels == 4) ? GL_RGBA : GL_RGB;
            GLenum dataFormat = (image.channels == 4) ? GL_RGBA : GL_RGB;
            GLenum dataType = GL_UNSIGNED_BYTE;

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, atlasWidth, atlasHeight, 0, dataFormat, dataType, image.pixels.data());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            if (generateMipmaps) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            glBindTexture(GL_TEXTURE_2D, 0);  // Unbind after initialization
        }
    };
} // namespace almond

#endif
