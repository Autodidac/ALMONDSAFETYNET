#version 460 core

layout(location = 0) in vec3 position;   // Vertex position
layout(location = 1) in vec2 texCoords;  // Texture coordinates

out vec2 TexCoords;  // Pass texture coordinates to fragment shader

uniform vec2 positionOffset; // For positioning the mesh on the screen
uniform bool isAtlas;        // To check if we're using a texture atlas or single texture
uniform vec2 texOffset;      // Offset for texture atlas
uniform vec2 texSize;        // Size of the individual texture in the atlas
uniform vec2 scale;          // For scaling the mesh

void main()
{
    // Apply position offset and scaling
    gl_Position = vec4((position + vec3(positionOffset, 0.0)) * vec3(scale, 1.0), 1.0);

    // Calculate the final texture coordinates based on the texture atlas or regular texture
    if (isAtlas) {
        // For texture atlas, adjust the texCoords with the offset and size of the sub-texture
        TexCoords = texCoords * texSize + texOffset;
    } else {
        // For regular textures, just pass the texCoords
        TexCoords = texCoords;
    }
}
