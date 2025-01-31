#pragma once

#include "alsTexturePool.h"
#include <tuple>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

namespace almond {
namespace SpriteBank {
    using Sprite = std::tuple<almond::texturepool::Texture, float, float, float, float, int, int>; // (Texture, uMin, vMin, uMax, vMax, width, height)

    // Lazy-loaded sprite bank (sprite name -> sprite data)
    inline auto& getBank() {
        static std::unordered_map<std::string, Sprite> bank;
        return bank;
    }

    // Add a sprite to the bank
    inline void addSprite(const std::string& name, const std::filesystem::path& filepath,
        float uMin, float vMin, float uMax, float vMax, int width, int height) {
        auto texture = TexturePool::loadTexture(filepath);
        auto& bank = getBank();

        if (!bank.emplace(name, Sprite{ texture, uMin, vMin, uMax, vMax, width, height }).second) {
            throw std::runtime_error("Sprite with name '" + name + "' already exists.");
        }

        std::cout << "Added sprite: " << name << " from texture: " << filepath << "\n";
    }

    // Get a sprite from the bank
    inline auto getSprite(const std::string& name) -> const Sprite& {
        auto& bank = getBank();
        if (auto it = bank.find(name); it != bank.end()) {
            return it->second;
        }
        throw std::runtime_error("Sprite with name '" + name + "' not found.");
    }

    // Remove a sprite from the bank
    inline void removeSprite(const std::string& name) {
        auto& bank = getBank();
        if (bank.erase(name)) {
            std::cout << "Removed sprite: " << name << "\n";
        }
    }

    // Clear all sprites from the bank
    inline void clear() {
        auto& bank = getBank();
        bank.clear();
        std::cout << "Cleared all sprites.\n";
    }
}
}