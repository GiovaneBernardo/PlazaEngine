#version 330 core

// Input vertex positions and texture coordinates
in vec3 position;
in vec2 texCoord;

// Output texture coordinates
out vec2 TexCoord;

void main()
{
    // Pass the vertex position to the fragment shader
    gl_Position = vec4(position, 1.0);

    // Pass the texture coordinates to the fragment shader
    TexCoord = texCoord;
}
