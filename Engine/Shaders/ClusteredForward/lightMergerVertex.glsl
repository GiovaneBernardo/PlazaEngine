#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
uniform mat4 view;

out vec2 TexCoords;

vec2 screenSize = vec2(1820, 720);

vec3 clusterSize = vec3(32, 32, 32);

int lightCount = 0;
void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}
