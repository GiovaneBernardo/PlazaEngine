#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(pow(vec3(1.0f, 0.43f, 0.11f), vec3(2.2f)), 1.0);
}
