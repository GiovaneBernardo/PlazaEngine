#version 460
#extension GL_EXT_multiview : enable

layout(triangles, invocations = 10) in;
layout(triangle_strip, max_vertices = 3) out;

layout (binding = 0) uniform UBO
{
	mat4 lightSpaceMatrices[32];
} ubo;

//layout(location = 0) in mat4 lightSpaceMatrices[1088];

void main()
{
    for (int i = 0; i < 3; ++i)
    {
		gl_Position = ubo.lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
    }
    EndPrimitive();
}
