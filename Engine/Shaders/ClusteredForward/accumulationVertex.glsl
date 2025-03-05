#version 430 core
layout (location = 0) in vec3 aPos;
out vec3 FragPos;  // Pass fragment position to the fragment shader
out vec3 pos;

void main() {
    FragPos = aPos;
    pos = aPos;
    gl_Position = vec4(aPos, 1.0);
}
