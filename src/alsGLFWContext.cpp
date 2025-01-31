
#include "alsGLFWContext.h"
#include "alsEngineConfig.h"

#ifdef ALMOND_USING_GLFW

#ifdef ALMOND_USING_OPENGLTEXTURE
    #include "alsTexture.h"
    #include "alsOpenGLTexture.h" // OpenGL texture manager
    #include "alsOpenGLRenderer.h"
    #include "alsOpenGLTextureAtlas.h"
#endif

#include "alsGLFWSandSim.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <vector>
#include<filesystem>

namespace almond {

    GLFWwindow* glfwWindow = nullptr;

    int width = 1024;
    int height = 768;
    int posx;
    int posy;
    int oldposx;
    int oldposy;
    bool isButtonHeld = false;
    bool isAtlas = false;
    SandSimulation sandSim(width, height);

    std::vector<std::shared_ptr<almond::RenderCommand>> commands;
    OpenGLTexture* texture = nullptr;
    OpenGLTextureAtlas* textureA = nullptr;
    std::shared_ptr<Quad> quad = nullptr;
    Renderer* renderer = nullptr;
    glm::vec2 texOffset = glm::vec2(0.0f, 256.0f);
    glm::vec2 texSize = glm::vec2(1.0f, 1.0f);


    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            std::cout << "Key Pressed: " << key << "\n";
        }
        else if (action == GLFW_RELEASE) {
            std::cout << "Key Released: " << key << "\n";
        }

        if (action == GLFW_KEY_SPACE) {
            //isGameOfLife = !isGameOfLife;
            //isSnakeGame = false;
        }
    }

    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (action == GLFW_PRESS) isButtonHeld = true;
        if (action == GLFW_RELEASE) isButtonHeld = false;
    }

    void ProcessInput(int x, int y, int radius) {
        if (isButtonHeld) {
            // Sand brush logic here
            for (int offsetY = -radius; offsetY <= radius; ++offsetY) {
                for (int offsetX = -radius; offsetX <= radius; ++offsetX) {
                    if (offsetX * offsetX + offsetY * offsetY <= radius * radius) {
                        int gridX = std::clamp(x + offsetX, 0, width - 1);
                        int gridY = std::clamp(y + offsetY, 0, height - 1);
                        sandSim.addSandAt(gridX, gridY);
                    }
                }
            }
        }
    }

    void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {

        //GLFW_CURSOR
        if (isButtonHeld)
        {
            posx = static_cast<int>(xpos);
            posy = static_cast<int>(ypos);
            if(xpos != oldposx || ypos != oldposy)
            {
                std::cout << "Cursor Position: (" << xpos << ", " << ypos << ")\n";
                oldposx = static_cast<int>(xpos);
                oldposy = static_cast<int>(ypos);

            }
        }
    }

    void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
        std::cout << "Window resized: " << width << "x" << height << "\n";
        glViewport(0, 0, width, height); // Update OpenGL viewport
    }

    void initGLFW() {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwWindow = glfwCreateWindow(width, height, "GLFW with OpenGL", nullptr, nullptr);
        if (!glfwWindow) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(glfwWindow);

        // important step
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // glfwSwapInterval(1); // Enable vsync
        glViewport(0, 0, width, height); // Update OpenGL viewport

        //        const char* version = (const char*)glGetString(GL_VERSION);
        //        std::cout << "OpenGL Version: " << version << std::endl;

        glfwSetKeyCallback(glfwWindow, KeyCallback);
        glfwSetMouseButtonCallback(glfwWindow, MouseButtonCallback);
        glfwSetCursorPosCallback(glfwWindow, CursorPositionCallback);
        glfwSetFramebufferSizeCallback(glfwWindow, FramebufferSizeCallback);

        std::cout << "GLFW and OpenGL initialized successfully\n";
    }

    bool processGLFW() 
    {
        if (glfwWindowShouldClose(glfwWindow)) { return false; }

        // Initialize global OpenGL states
        glClearColor(0.2f, 0.2f, 0.2f, 0.1f);  // Set background color once
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Enable blending once

        auto lastTime = std::chrono::steady_clock::now();
        int frameCount = 0;

        //grab the grid
        const auto& grid = sandSim.getGrid();

        // Setup Texture Path
        const std::filesystem::path& texturePath = std::filesystem::current_path().parent_path().parent_path() / "assets" / "images" / "bmp3.bmp";

        // Define vertices and indices
        std::vector<float> vertices = {
           // Vertex Positions  Texture Coords
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
        };

        std::vector<unsigned int> indices = {
            0, 1, 2,
            2, 3, 0
        };

        isAtlas = true;
        // Texture Atlas Setup
        if(isAtlas == true)
        {
            OpenGLTextureAtlas textureAtlas(texturePath, Texture::Format::RGBA8, true, 256, 256);

            static almond::Renderer localRenderer(textureAtlas, "../../assets/shaders/vert.glsl", "../../assets/shaders/frag.glsl"); // Create a renderer object for the texture atlas;  // Must be static if you want to persist beyond this block
            localRenderer.SetRenderMode(RenderMode::TextureAtlas);
            renderer = &localRenderer;  // Use reference to access outside the block
            //localRenderer.AddTexture(texturePath, 0, 0);
           // localRenderer.AddTexture(texturePath.string().c_str(), 256, 0);
           // localRenderer.AddTexture(texturePath.string().c_str(), 0, 256);

            // *renderer = &renderer
            localRenderer.AddQuad("sand_quad", vertices, indices); // Create a new mesh object in the renderer for our quad
            //        auto mesh = renderer.GetMesh("sand_quad");
            quad = localRenderer.GetQuad("sand_quad");

            // "C:/Users/iammi/OneDrive/Documents/repos/AlmondShell/AlmondShell/assets/images/bmp3.bmp"
            textureA = &localRenderer.GetTextureAtlas(texturePath);
        }
        else
        {
            static Renderer localRenderer("../../assets/shaders/vert.glsl", "../../assets/shaders/frag.glsl"); // Create a renderer object for the texture 
            renderer = &localRenderer;  // Use reference to access outside the block

            localRenderer.AddQuad("sand_quad", vertices, indices); // Create a new mesh object in the renderer for our quad
            //        auto mesh = renderer.GetMesh("sand_quad");
           quad = localRenderer.GetQuad("sand_quad");

           localRenderer.AddTexture(texturePath, 0, 256); // only path matters to single texture
           localRenderer.SetRenderMode(RenderMode::SingleTexture);
           texture = &localRenderer.GetTexture("C:/Users/iammi/OneDrive/Documents/repos/AlmondShell/AlmondShell/assets/images/bmp3.bmp");
            
        }
        //  *texture = renderer.GetTexture("C:/Users/iammi/OneDrive/Documents/repos/AlmondShell/AlmondShell/assets/images/bmp3.bmp");
        commands.push_back(std::make_unique<almond::SetVec2Uniform>("scale", glm::vec2(1.0f, 1.0f)));
        
        // Enable wireframe mode
       //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
/*
        // Set up a projection matrix
        glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Load the font
        FontManager fontManager("../../assets/fonts/KawayjampersonaluseRegular-DOj8m.otf");

        GLuint fontVAO = 0;
        GLuint fontVBO = 0;
        GLuint fontEBO = 0;
        renderer.InitializeFontRenderingResources(fontVAO, fontVBO, fontEBO);
        FontRenderer fontRenderer(fontManager, renderer, fontVAO, fontVBO, fontEBO);
        fontRenderer.RenderText("Almond Shell by Adam Rushford", 25.0f, 570.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
*/
        while (!glfwWindowShouldClose(glfwWindow)) {
            auto currentTime = std::chrono::steady_clock::now();
            frameCount++;

            // Calculate FPS
            std::chrono::duration<float> elapsedTime = currentTime - lastTime;
            if (elapsedTime.count() >= 1.0f) {
                std::cout << "FPS: " << frameCount << "\n";
                frameCount = 0;
                lastTime = currentTime;
            }

            // Clear screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Process input
            ProcessInput(0, 0, 4);
            if(isAtlas == true)
            {
                // Update simulation
                sandSim.update();

                // Initialize and fill activeParticles with the positions of active particles
                std::vector<std::pair<int, int>> activeParticles;
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        if (grid[y * width + x] > 0) {
                            activeParticles.emplace_back(x, y);
                        }
                    }
                }

                if (renderer->GetRenderMode(almond::RenderMode::TextureAtlas))
                {
                    // Render particles
                    if (quad) {
                        for (const auto& particle : activeParticles) {
                            float xpos = (float)particle.first / width * 2.0f - 1.0f; // Normalize to [-1, 1] range
                            float ypos = (float)particle.second / height * 2.0f - 1.0f; // Normalize to [-1, 1] range
                            commands.push_back(std::make_shared<almond::DrawCommand>(quad, 0, almond::RenderMode::TextureAtlas, xpos, ypos));//, texOffset, texSize
                        }
                    }
                }

                renderer->DrawBatch(quad, *textureA, commands);
            }
            else {
                // A Single Textured Quad Rendered to Screen
                if(renderer->GetRenderMode(almond::RenderMode::SingleTexture))
                {
                    if (quad) {
                        // uncomment for single texture no atlas
                        renderer->DrawSingleQuad(quad, *texture, 0.0f, 0.0f);

                    }
                }
            }
           // fontRenderer.RenderText("Almond Shell by Adam Rushford", 25.0f, 570.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

            // Swap buffers and poll events
            glfwSwapBuffers(glfwWindow);
            glfwPollEvents();
        }

        return true;
    }

    void cleanupGLFW() {
        if (glfwWindow) {
            glfwDestroyWindow(glfwWindow);
            glfwWindow = nullptr;
        }

        //sandSim.cleanup(); // Cleanup sand simulation resources
        glfwTerminate();
    }
}

#endif
