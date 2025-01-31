#pragma once

#include "alsEngineConfig.h"
#include "alsImageLoader.h"
#include "alsTexture.h"

#ifdef ALMOND_USING_OPENGLTEXTURE

#include <stdexcept>
#include <iostream>
#include <memory>

namespace almond {
    void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        std::cerr << "OpenGL Debug Output: " << message << std::endl;
    }

    class OpenGLTexture : public almond::Texture {
    public:
        OpenGLTexture() {}
        OpenGLTexture(const std::filesystem::path& filepath, Format format, bool generateMipmaps = true)
            : filepath(filepath), format(format), generateMipmaps(generateMipmaps)  {
            LoadTexture(filepath);

            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(DebugCallback, nullptr);
        }

        ~OpenGLTexture() override {
            if (id != 0) {
                glDeleteTextures(1, &id);
            }
        }

        bool IsValid() const {
            return glIsTexture(id) == GL_TRUE;
        }

        void Bind(unsigned int slot = 0) const override {
            if (id == 0 || !IsValid()) {
                std::cerr << "Error: Attempted to bind an invalid texture with ID: " << id << "\n";
                return;
            }

            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, id);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cerr << "OpenGL Error during texture binding: " << error << "\n";
            }
        }

        void Unbind() const override {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        int GetWidth() const override { return width; }
        int GetHeight() const override { return height; }
        unsigned int GetID() const override { return id; }
        std::filesystem::path GetPath() const override { return filepath; }

        void SetFiltering(GLenum minFilter, GLenum magFilter) const override {
            glBindTexture(GL_TEXTURE_2D, id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        std::vector<unsigned char> GetData() const override {
            glBindTexture(GL_TEXTURE_2D, id);
            std::vector<unsigned char> data(width * height * 4);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
            return data;
        }

    private:
        GLuint id = 0;
        int width = 0;
        int height = 0;
        Format format = almond::Texture::Format::RGBA8;
        bool generateMipmaps = true;
        ImageLoader::ImageData image;
        const std::filesystem::path filepath = "";

        void FlipImageVertically(unsigned char* data, int width, int height, int channels) {
            int rowSize = width * channels;
            std::vector<unsigned char> rowBuffer(rowSize);

            for (int i = 0; i < height / 2; ++i) {
                unsigned char* topRow = data + i * rowSize;
                unsigned char* bottomRow = data + (height - i - 1) * rowSize;

                // Swap the rows
                std::memcpy(rowBuffer.data(), topRow, rowSize);
                std::memcpy(topRow, bottomRow, rowSize);
                std::memcpy(bottomRow, rowBuffer.data(), rowSize);
            }
        }

        void LoadTexture(const std::filesystem::path& filepath) {
            image = ImageLoader::LoadAlmondImage(filepath.string().c_str());

            if (image.pixels.empty()) {
                throw std::runtime_error("Failed to load image: " + filepath.string());
            }

            width = image.width;
            height = image.height;
            // Flip the image
            FlipImageVertically(image.pixels.data(), width, height, image.channels);
            std::cout << "Loaded texture: " << filepath.string() << " (" << width << "x" << height << ")" << std::endl;

            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);

            GLenum internalFormat = (image.channels == 4) ? GL_RGBA : GL_RGB;
            GLenum dataFormat = (image.channels == 4) ? GL_RGBA : GL_RGB;
            GLenum dataType = GL_UNSIGNED_BYTE;

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, dataType, image.pixels.data());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Generate mipmaps if needed
            glGenerateMipmap(GL_TEXTURE_2D);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                throw std::runtime_error("OpenGL error during texture creation: " + std::to_string(error));
            }

            std::cout << "Texture loaded successfully." << std::endl;
        }

    };
}

#endif
