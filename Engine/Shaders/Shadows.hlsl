#pragma extension SPV_KHR_multiview

#define MAX_BONES 1000
#define MAX_BONE_INFLUENCE 4

struct VSInput {
    float3 aPos              : TEXCOORD0;
    float4 instanceMatrix0   : TEXCOORD1;
    float4 instanceMatrix1   : TEXCOORD2;
    float4 instanceMatrix2   : TEXCOORD3;
    float4 instanceMatrix3   : TEXCOORD4;
    // int4 boneIds           : TEXCOORD9;
    // float4 weights         : TEXCOORD10;
};

// Output to pixel shader
struct VSOutput {
    float2 uv     : TEXCOORD0;
    float4 pos    : SV_POSITION;
};

// Uniform Buffer Object
[[vk::binding(0, 0)]]
cbuffer ShadowsUBO
{
    row_major float4x4 lightSpaceMatrices[32];
};

// StructuredBuffer for bone matrices
[[vk::binding(1, 0)]]
StructuredBuffer<float4x4> BoneMatrices;

// Main VS
VSOutput mainVS(VSInput input, uint viewID : SV_ViewID)
{
    VSOutput output;

    row_major float4x4 model;
    model[0] = input.instanceMatrix0;
    model[1] = input.instanceMatrix1;
    model[2] = input.instanceMatrix2;
    model[3] = input.instanceMatrix3;

    float4 totalPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    bool allNegative = true;

    // Commented out version of skinning loop
    /*
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (input.boneIds[i] < 0 || input.boneIds[i] > 1000)
            continue;
        else
            allNegative = false;

        if (input.boneIds[i] >= MAX_BONES)
        {
            totalPosition = float4(input.aPos, 1.0f);
            break;
        }

        float4 localPos = mul(boneMatrices[input.boneIds[i]], float4(input.aPos, 1.0f));
        totalPosition += localPos * input.weights[i];
    }
    */

    if (allNegative)
        totalPosition = float4(input.aPos, 1.0f);

    float4 worldPos = mul(model, totalPosition);
    output.pos = mul(lightSpaceMatrices[1], worldPos);
    //output.viewID = viewID;
    output.uv = input.aPos.xy; // Dummy UV since original GLSL didn't have UV
    return output;
}

struct PSInput {
    float2 uv : TEXCOORD0;
};

//struct PSOutput {
//    float4 ShadowsDepthMap    : SV_Target0;
//};

void mainPS(PSInput input, uint viewID : SV_ViewID)
{
  //  PSOutput output;
   // output.ShadowsDepthMap   = float4(1, 0, 0, 1);
   // return output;
}