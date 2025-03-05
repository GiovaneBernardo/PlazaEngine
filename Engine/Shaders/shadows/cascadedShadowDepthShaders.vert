#version 460
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 aPos;
layout(location = 5) in vec4 instanceMatrix[4];
//layout(location = 8) in ivec4 boneIds;
//layout(location = 9) in vec4 weights;
layout(location = 0) out vec2 outUV;

const int MAX_BONES = 1000;
const int MAX_BONE_INFLUENCE = 4;
layout(std430, binding = 1) readonly buffer BoneMatrices {
	mat4 boneMatrices[];
};

layout (binding = 0) uniform UBO
{
	mat4 lightSpaceMatrices[32];
} ubo;

out gl_PerVertex {
	vec4 gl_Position;
};

const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

void main()
{
    mat4 model = mat4(instanceMatrix[0], instanceMatrix[1], instanceMatrix[2], instanceMatrix[3]);

    vec4 totalPosition = vec4(0.0f);
    bool allNegative = true;

   //for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
   //{
   //      if(boneIds[i] < 0 || boneIds[i] > 1000)
   //      {
   //        continue;
   //      }
   //      else
   //      {
   //           allNegative = false;
   //      }
   //      if(boneIds[i] >= MAX_BONES)
   //      {
   //          totalPosition = vec4(aPos,1.0f);
   //          break;
   //      }
   //      vec4 localPosition = boneMatrices[boneIds[i]] * vec4(aPos,1.0f);
   //      totalPosition += localPosition * weights[i];
   //}

     if(allNegative)
         totalPosition = vec4(aPos,1.0f);

	vec4 pos = model * totalPosition;
    gl_Position = ubo.lightSpaceMatrices[gl_ViewIndex] * pos;
}
