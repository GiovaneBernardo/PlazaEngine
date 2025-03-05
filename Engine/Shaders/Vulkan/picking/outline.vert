#version 450
layout (location = 0) in vec3 inPos;
layout(location = 5) in vec4 instanceMatrix[4];
layout(location = 0) out vec3 fragTexCoord;

layout(push_constant) uniform PushConstants{
    mat4 projection;
    mat4 view;
} pushConstants;

void main()
{
	fragTexCoord = inPos;

	mat4 viewMat = mat4(mat3(pushConstants.view));
	gl_Position = pushConstants.projection * viewMat * vec4(inPos.xyz, 1.0);
}
