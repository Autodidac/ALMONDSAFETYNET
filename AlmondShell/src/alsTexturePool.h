#pragma once



#include <filesystem>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <iostream>

namespace almond {
    namespace texturepool {
        using Texture = std::shared_ptr<std::string>;

        // Lazy-loaded texture map (file path -> texture)
        inline auto& getPool() {
            static std::unordered_map<std::string, Texture> pool;
            return pool;
        }

        // Load a texture or fetch it from the pool
        inline auto loadTexture(const std::filesystem::path& filepath) -> Texture {
            auto& pool = getPool();
            const std::string path = filepath.string();

            if (auto it = pool.find(path); it != pool.end()) {
                return it->second; // Texture already loaded
            }

            // Simulate loading a texture
            auto texture = std::make_shared<std::string>("Texture: " + path);
            pool[path] = texture;
            std::cout << "Loaded texture: " << path << "\n";
            return texture;
        }

        // Remove a texture from the pool
        inline void releaseTexture(const std::filesystem::path& filepath) {
            auto& pool = getPool();
            const std::string path = filepath.string();

            if (pool.erase(path)) {
                std::cout << "Released texture: " << path << "\n";
            }
        }

        // Clear all textures from the pool
        inline void clear() {
            auto& pool = getPool();
            pool.clear();
            std::cout << "Cleared all textures.\n";
        }
    }
}