#pragma once

#include "alsEngineConfig.h"
#include "alsOpenGLMesh.h"
#include "alsOpenGLShader.h"
#include "alsOpenGLQuad.h"

#ifdef ALMOND_USING_GLFW

#include <optional>
#include <memory>
#include <cassert>
#include <iostream>

namespace almond {

    enum class RenderMode {
        SingleTexture,
        TextureAtlas
    };

    enum class DrawMode {
        QUADS,  // Draw with quads (for font rendering, etc.)
        MESH    // Draw with meshes (for 3D models, etc.)
    };

    class RenderCommand {
    public:
        enum class CommandType {
            Draw,
            SetUniform
        };

        explicit RenderCommand(CommandType type) : type(type) {}
        virtual ~RenderCommand() = default;

        CommandType GetType() const { return type; }
        virtual void Execute(ShaderProgram* shader) const = 0;

    private:
        CommandType type;
    };

    class UniformCommand : public RenderCommand {
    public:
        explicit UniformCommand(const std::string& name)
            : RenderCommand(CommandType::SetUniform), uniformName(name) {}

        virtual ~UniformCommand() = default;

        std::string GetName() const { return uniformName; }

    protected:
        std::string uniformName;
    };

    class SetVec2Uniform : public UniformCommand {
    public:
        SetVec2Uniform(const std::string& name, const glm::vec2& value)
            : UniformCommand(name), value(value) {}

        void Execute(ShaderProgram* shader) const override {
            shader->SetUniform(uniformName, value);
        }

    private:
        glm::vec2 value;
    };

    class DrawCommand : public RenderCommand {
    public:

       DrawCommand(std::shared_ptr<Quad> quad, int textureSlot, RenderMode renderMode, float x, float y)
            : RenderCommand(CommandType::Draw), quad(std::move(quad)), textureSlot(textureSlot), renderMode(renderMode), x(x), y(y) {}

         DrawCommand(std::shared_ptr<Quad> quad, int textureSlot, RenderMode renderMode, float x, float y, glm::vec2 texOffset, glm::vec2 texSize)
            : RenderCommand(CommandType::Draw), quad(std::move(quad)), textureSlot(textureSlot), renderMode(renderMode), x(x), y(y), texOffset(texOffset), texSize(texSize) {}
/*
       DrawCommand(std::shared_ptr<Mesh> mesh, int textureSlot, RenderMode renderMode, float x, float y, glm::vec2 texOffset, glm::vec2 texSize)
            : RenderCommand(CommandType::Draw), mesh(std::move(mesh)), textureSlot(textureSlot), renderMode(renderMode), x(x), y(y), texOffset(texOffset), texSize(texSize) {}
*/
         void Execute(ShaderProgram* shader) const override {
            assert(shader && "Shader program must not be null");

            // Set texture atlas uniforms if necessary
            SetTextureAtlasUniforms(shader);

            if (quad) {
                
                glBindVertexArray(quad->GetVAO());
                glDrawElements(GL_TRIANGLES, quad->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            } else if (mesh) {
                glBindVertexArray(mesh->GetVAO());
                glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            } else {
                std::cerr << "ERROR: Neither mesh nor quad is valid." << std::endl;
            }

            glBindVertexArray(0);
        }

        const std::shared_ptr<Mesh>& GetMesh() const { return mesh; }
        const std::shared_ptr<Quad>& GetQuad() const { return quad; } // wrong get the quad from the renderer.. somehow
        int GetTextureSlot() const { return textureSlot; }

    private:
        void SetTextureAtlasUniforms(ShaderProgram* shader) const {
            if (renderMode == RenderMode::TextureAtlas) {
                shader->SetUniform("isAtlas", true);
                shader->SetUniform("texOffset", texOffset);
                shader->SetUniform("texSize", texSize);
            } else {
                shader->SetUniform("isAtlas", false);
                shader->SetUniform("texOffset", glm::vec2(0.0f, 0.0f));
                shader->SetUniform("texSize", glm::vec2(1.0f, 1.0f));
            }
        }

        std::shared_ptr<Quad> quad;
        std::shared_ptr<Mesh> mesh;
        int textureSlot;
        float x, y;
        RenderMode renderMode;
        glm::vec2 texOffset{ 0.0f, 0.0f };
        glm::vec2 texSize{ 0.0f, 0.0f };
    };

} // namespace almond

#endif
