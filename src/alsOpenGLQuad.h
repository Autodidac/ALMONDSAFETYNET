#pragma once

#include "alsEngineConfig.h"

#ifdef ALMOND_USING_GLFW
#include <vector>
#include <memory>

namespace almond {

    class Quad {
    public:
        Quad(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
        ~Quad();

        GLuint GetVBO() const { return VBO; }
        GLuint GetEBO() const { return EBO; }
        GLuint GetVertexCount() const { return vertexCount; }
        GLuint GetIndexCount() const { return indexCount; }

        bool HasVAO() const { return VAO != 0; }
        GLuint GetVAO() const { return VAO; }
        void SetVAO(unsigned int vao) { VAO = vao; }

        void setTexture(unsigned int textureID) { this->textureID = textureID; }
        GLuint getTexture() const { return textureID; }

        void Draw() const;

    private:
        GLuint VAO = 0;
        GLuint VBO = 0, EBO = 0;
        GLuint vertexCount = 0, indexCount = 0;
        GLuint textureID = 0;

        void init(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    };

}
#endif
