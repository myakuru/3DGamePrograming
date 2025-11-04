#include "inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

cbuffer cb : register(b0)
{
	float g_threshold;
};

float4 main(VSOutput In) : SV_Target0
{
    float4 texColor = g_inputTex.Sample(g_ss, In.UV);

    texColor.rgb = max(0.0f , texColor.rgb - g_threshold);

    return texColor;
}
