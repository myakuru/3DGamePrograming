#include "../inc_KdCommon.hlsli"
#include "inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

// 放射ブラー用パラメータ
cbuffer cbRadial : register(b4)
{
	float g_strength;		// 伸ばし強度
	float g_sampleCount;	// サンプリング数
	float2 g_uvOffset;		// UVオフセット
}

float4 main(VSOutput In) : SV_Target0
{
	float3 color = float3(0.0f, 0.0f,0.0f);
	float2 uv = In.UV - g_uvOffset;
	// ピクセルが中心から遠いほど大きく伸ばす
	float factor = g_strength / g_sampleCount * length(uv);

	// 現在のピクセルからブラー中心方向へ少しずつ寄った複数地点をサンプリングして色を加算
	for(int i = 0; i < g_sampleCount; i++)
	{
		// サンプル数が増えるほどオフセットが大きくなる
		float offset = 1 - factor * i;
		color += g_inputTex.Sample(g_ss, uv * offset + g_uvOffset).rgb;
	}

	// 色を戻すために平均化して返す
	return float4(color, 1) / g_sampleCount;
}
