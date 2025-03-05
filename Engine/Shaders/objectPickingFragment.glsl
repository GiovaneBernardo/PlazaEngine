#version 330 core
#extension GL_NV_gpu_shader5 : enable
uniform uint64_t objectID;
out uint64_t pixelObjectID;

void main(){
    pixelObjectID = objectID;
}
