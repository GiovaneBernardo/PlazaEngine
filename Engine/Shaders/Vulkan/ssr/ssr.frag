#version 450

layout (binding = 0) uniform sampler2D normalTexture;
layout (binding = 1) uniform sampler2D sceneTexture;
layout (binding = 2) uniform sampler2D othersTexture;
layout (binding = 3) uniform sampler2D depthTexture;

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
    vec4 screenSize;
    vec4 cameraPos;
    mat4 projection;
    mat4 view;
    mat4 lensProjection;
} pushConstants;

float GetDepthUV(vec2 uv) {
//uv.y = 1.0f - uv.y;
    return texture(depthTexture, uv).r;
}

vec3 ReconstructWorldPosition(vec2 uv) {
    mat4 viewProjection = pushConstants.projection * pushConstants.view;
    float ndcZ = GetDepthUV(uv) * 2.0f - 1.0f;
    vec4 ndcPosition = vec4(uv * 2.0 - 1.0, (GetDepthUV(uv)), 1.0);
    ndcPosition.y *= -1.0f;
    vec4 worldPosition = inverse(viewProjection) * ndcPosition;
    worldPosition.xyz /= worldPosition.w;
    return worldPosition.xyz;
    //vec4 clipPos = vec4(uv * 2.0f - 1.0f, depth, 1.0);
    //vec4 viewPos = inverse(pushConstants.view) * clipPos;
    //return viewPos.xyz / viewPos.w;
}

vec3 ReconstructViewPosition(vec2 uv) {
    mat4 viewProjection = pushConstants.projection * pushConstants.view;
    vec4 clipPos = vec4(uv * 2.0 - 1.0, GetDepthUV(uv), 1.0);
    //clipPos.y *= -1.0f;
    vec4 viewPos = inverse(viewProjection) * clipPos;
    return viewPos.xyz / viewPos.w;
}


float GetDepth() {
    return texture(depthTexture, inUV).r;
}

//vec3 ReconstructViewPosition(vec2 uv, float depth) {
//    vec4 clipPos = vec4(uv * 2.0f - 1.0f, depth, 1.0);
//     clipPos.y *= -1;
//    vec4 viewPos = inverse(pushConstants.view) * clipPos;
//    return viewPos.xyz / viewPos.w;
//}

vec3 WorldToViewSpace(vec3 worldPosition) {
    vec4 viewSpacePosition = pushConstants.view * vec4(worldPosition, 1.0);
    return viewSpacePosition.xyz;
}

vec4 WorldToClipSpace(vec3 worldPosition) {
    vec4 clipSpacePosition = pushConstants.projection * pushConstants.view * vec4(worldPosition, 1.0);
    return clipSpacePosition;
}

vec3 ClipToNDC(vec4 clipSpacePosition) {
    return clipSpacePosition.xyz / clipSpacePosition.w;
}

vec3 WorldToNDC(vec3 worldPosition) {
    vec4 clipSpacePosition = pushConstants.projection * pushConstants.view * vec4(worldPosition, 1.0);
    return clipSpacePosition.xyz / clipSpacePosition.w;
}

vec3 reconstructViewPos(float depth, vec2 uv, mat4 invProj, vec2 screenSize) {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clipSpacePos = vec4(ndc, depth, 1.0);
    clipSpacePos.y *= -1.0f;
    vec4 viewSpacePos = invProj * clipSpacePos;
    viewSpacePos /= viewSpacePos.w;
    return viewSpacePos.xyz;
}

void main() {
    fragColor = vec4(texture(sceneTexture, inUV).xyz, 1.0f);
	return;

    vec2 texSize = pushConstants.screenSize.xy;
    vec2 texCoord = clamp(inUV, vec2(0.0), vec2(1.0));
    float sceneDepth = GetDepthUV(texCoord);

    //if (sceneDepth >= 1.0f || 1.0f - texture(othersTexture, inUV).y < 0.4f) {
        fragColor = texture(sceneTexture, inUV);
        return;
    //}

    vec3 viewPos = reconstructViewPos(sceneDepth, texCoord, inverse(pushConstants.projection), texSize);
    vec3 normal = normalize(texture(normalTexture, texCoord).xyz);
    vec3 unitViewPos = normalize(viewPos);

    vec4 ndcPos = vec4(viewPos, 1.0f);
    vec3 worldPos = (inverse(pushConstants.view) * ndcPos).xyz;
    mat3 normalMatrix = transpose((mat3(pushConstants.view)));
    vec3 worldNormal = normalize(normalMatrix * normal);

    vec3 cameraPos = pushConstants.cameraPos.xyz;
vec3 unitWorldViewPos = normalize(worldPos - cameraPos);
vec3 worldPivot = normalize(reflect(unitWorldViewPos, worldNormal));

mat3 piv = mat3(pushConstants.view);
vec3 pivot = ((piv) * worldPivot);


    float maxDistance = 8.0;
    float resolution = 0.3;
    int steps = 4;
    float thickness = 0.5;

    vec4 startView = vec4(viewPos, 1.0);
    vec4 endView = vec4(viewPos + pivot * maxDistance, 1.0);

    vec4 startFrag = (pushConstants.projection * startView);
    startFrag.xyz /= max(startFrag.w, 0.0001);
    startFrag.xy = (startFrag.xy * 0.5 + 0.5) * texSize;

    vec4 endFrag = (pushConstants.projection * endView);
    endFrag.xyz /= max(endFrag.w, 0.0001);
    endFrag.xy = (endFrag.xy * 0.5 + 0.5) * texSize;

    vec2 frag = startFrag.xy;
    vec2 increment = (endFrag.xy - startFrag.xy) / float(steps);

    float viewDistance = reconstructViewPos(GetDepthUV(inUV), inUV, inverse(pushConstants.projection), texSize).z;//sceneDepth;
    float depth = 0.0;
    bool hit = false;

    vec3 viewPosTo = viewPos;
    for (int i = 0; i < steps; ++i) {
        frag += increment;
        vec2 uv = frag / texSize;

        uv.y = 1.0f - uv.y;
        uv = clamp(uv, vec2(0.0), vec2(1.0));

        viewPosTo = reconstructViewPos(GetDepthUV(uv), uv, inverse(pushConstants.projection), texSize);
        depth = viewDistance - viewPosTo.z;

        if (depth > 0.0 && depth < thickness) {
            hit = true;
            break;
        }
    }

    float visibility = 1.0;
    if (hit) {
       visibility = 1.0
          * (1.0 - max(dot(-unitViewPos, pivot), 0.0)) // Angle effect
           * (1.0 - clamp(depth / thickness, 0.0, 1.0)) // Thickness effect
           * (1.0 - clamp(length(viewPosTo - viewPos) / maxDistance, 0.0, 1.0)); // Distance effect
    }

    visibility = clamp(visibility, 0.0, 1.0);
    vec3 baseColor = texture(sceneTexture, texCoord).rgb;
    frag = frag / texSize;
    frag.y = 1.0f - frag.y;
    if(frag.y > 1.0f || frag.y < 0.0f)
        visibility = 0.0f;
    frag.y = clamp(frag.y, 0.0, 1.0);
    vec3 reflectionColor = texture(sceneTexture, frag).rgb;

    fragColor = vec4(baseColor + reflectionColor * visibility * 0.8, 1.0);
    //fragColor = vec4(vec3(depth), 1.0f);
    //fragColor = vec4(vec3(viewPosTo), 1.0f);
}
