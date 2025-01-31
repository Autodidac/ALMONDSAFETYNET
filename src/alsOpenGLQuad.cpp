#include "alsOpenGLQuad.h"

#ifdef ALMOND_USING_GLFW

namespace almond 
{
    Quad::~Quad(){}
    // Constructor to initialize the buffers
    Quad::Quad(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        init(vertices, indices);
    }

    void Quad::init(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        vertexCount = static_cast<unsigned int>(vertices.size() / 5);
        indexCount = static_cast<unsigned int>(indices.size());

        // Generate VAO, VBO, and EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinates attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Quad::Draw() const {
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

#endif
