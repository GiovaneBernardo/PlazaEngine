#version 450

layout (binding = 0) uniform sampler2D samplerTexture;
layout (binding = 1) uniform sampler2D fontTexture;

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

layout(push_constant) uniform PushConstants {
    float exposure; //1.2f
    float gamma; //2.6f
} pushConstants;
vec3 gammaCorrect(vec3 color) 
{
    return pow(color, vec3(1.0 / pushConstants.gamma));
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3 ACESInputMat =
{
    {0.59719, 0.07600, 0.02840},
    {0.35458, 0.90834, 0.13383},
    {0.04823, 0.01566, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat =
{
    { 1.60475, -0.10208, -0.00327},
    {-0.53108,  1.10813, -0.07276},
    {-0.07367, -0.00605,  1.07602 }
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}


vec3 acesFilm(const vec3 x) {
    const float a = 2.51* 2.0f;
    const float b = 0.03;
    const float c = 2.43 * 2.0f;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d ) + e), 0.0, 1.0);
}

vec3 filmicTonemap(vec3 color) {
    color = max(vec3(0.0), color - vec3(0.004)); // Linear section near black.
    return (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
}


void main() 
{
    vec4 x = texture(samplerTexture, inUV);
    vec3 color = x.rgb;
    color = filmicTonemap(color);
    color = gammaCorrect(color);
    color = clamp(color, 0.0, 1.0);
    outFragcolor = vec4(color, 1.0f);
    //vec4 x = pushConstants.exposure * texture(samplerTexture, inUV);
    //vec3 color = ACESInputMat * x.rgb;
    //     color = RRTAndODTFit(color);
    //     color = ACESOutputMat * color;
    //     color = gammaCorrect(color);
    //     color = clamp(color, 0.0, 1.0);
    //
   ////float contrastFactor = 0.5f;
   ////float lum = dot(color.rgb, vec3(0.299, 0.587, 0.114)); // Calculate luminance
   ////color.rgb += (color.rgb - lum) * contrastFactor;
    //
    //     //color = acesFilm(texture(samplerTexture, inUV).xyz);
    ////// Tone mapping
    ////vec3 color = texture(samplerTexture, inUV).xyz;
	////color = Uncharted2Tonemap(color * pushConstants.exposure);
	////color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	////// Gamma correction
	////color = pow(color, vec3(1.0f / pushConstants.gamma));
	//outFragcolor = vec4(color, 1.0f);
}