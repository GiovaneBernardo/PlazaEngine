#version 430

layout(rgba32f, binding = 0) uniform image2D image1;
layout(rgba32f, binding = 1) uniform image2D image2;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
    ivec2 texelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec4 color = imageLoad(image1, texelCoords);
    float threeshold = 0.5f;
    if(color.x > threeshold || color.y > threeshold || color.z > threeshold)
    {

    }
    else
    {
        color = vec4(0.0f);
    }


    imageStore(image2, ivec2(texelCoords), color);
}
