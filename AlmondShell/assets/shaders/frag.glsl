#version 460 core

in vec2 TexCoords;          // Texture coordinates from the vertex shader
out vec4 FragColor;         // Final color output

uniform sampler2D textureSampler; // The texture sampler (shared for images and glyphs)
uniform bool isAtlas;             // Indicates if using a texture atlas
uniform vec2 texOffset;           // Texture offset for atlas
uniform vec2 texSize;             // Size of the individual texture in the atlas
uniform bool isText;              // Indicates if rendering text
uniform vec4 textColor;           // Color of the text

void main()
{
    vec2 adjustedTexCoords = TexCoords;

    // Adjust texture coordinates if using a texture atlas
    if (isAtlas) {
        adjustedTexCoords = texOffset + TexCoords * texSize;
    }

    // Sample the texture
    vec4 sampled = texture(textureSampler, adjustedTexCoords);

    if (isText) {
        // For text, multiply the sampled alpha with the text color
        FragColor = vec4(textColor.rgb, textColor.a * sampled.r);
    } else {
        // For images, directly use the sampled color
        FragColor = sampled;
    }
}
