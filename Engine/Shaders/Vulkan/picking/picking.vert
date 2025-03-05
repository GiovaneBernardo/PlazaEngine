#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 5) in vec4 instanceMatrix[4];
layout(location = 0) out vec3 fragTexCoord;

layout(push_constant) uniform PushConstants{
    mat4 projection;
    mat4 view;
	mat4 model;
	uint uuid1;
	uint uuid2;
} pushConstants;


void main()
{
	vec4 finalModel = pushConstants.model * vec4(inPosition.xyz, 1.0);

	mat4 viewMat = mat4(mat3(pushConstants.view));
	gl_Position = pushConstants.projection * pushConstants.view * finalModel;
}
