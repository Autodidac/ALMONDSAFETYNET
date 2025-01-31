#pragma once

#include <memory>
#include <stdexcept>
#include <tuple>
#include <sstream>

namespace almond {
    struct Node {
        int x, y, width, height;
        bool used = false;
        std::unique_ptr<Node> right, down;

        Node(int x, int y, int width, int height)
            : x(x), y(y), width(width), height(height) {
        }
    };

    class TexturePacker {
        std::unique_ptr<Node> root;

    public:
        TexturePacker(int width, int height)
            : root(std::make_unique<Node>(0, 0, width, height)) {
        }

        std::tuple<int, int> Insert(int width, int height) {
            auto node = Insert(root.get(), width, height);
            if (!node) {
                std::ostringstream oss;
                oss << "Failed to insert texture ("
                    << width << "x" << height << ") into atlas ("
                    << root->width << "x" << root->height << ").";
                throw std::runtime_error(oss.str());
            }
            return { node->x, node->y };
        }

    private:
        Node* Insert(Node* node, int width, int height) {
            if (!node) return nullptr;

            if (node->used) {
                Node* result = Insert(node->right.get(), width, height);
                return result ? result : Insert(node->down.get(), width, height);
            }

            if (width > node->width || height > node->height) return nullptr;

            if (width == node->width && height == node->height) {
                node->used = true;
                return node;
            }

            // Split the node into right and down children
            node->right = std::make_unique<Node>(node->x + width, node->y, node->width - width, height);
            node->down = std::make_unique<Node>(node->x, node->y + height, node->width, node->height - height);

            node->width = width;
            node->height = height;
            node->used = true;

            return node;
        }

    };
}
