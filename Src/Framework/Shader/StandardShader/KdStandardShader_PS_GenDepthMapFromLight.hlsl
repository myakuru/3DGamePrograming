#include "inc_KdStandardShader.hlsli"

Texture2D g_tex : register(t0);

Texture2D g_dissolveTex : register(t11); // ディゾルブマップ

SamplerState g_ss : register(s0);

float4 main(VSOutputGenShadow  In) : SV_TARGET
{ 
	// ディゾルブによる描画スキップ
	float discardValue = g_dissolveTex.Sample(g_ss, In.UV).r;
	if (discardValue < g_dissolveValue)
	{
		discard;
	}

	// Alphaテスト：Alpha値が一定以下のピクセルは描画処理を飛ばす
	float4 texCol = g_tex.Sample(g_ss, In.UV);

	if (texCol.a * In.Color.a < 0.05f)
	{
		discard;
	}

	return float4(In.pPos.z / In.pPos.w, 0.0f, 0.0f, 1.0f);
}
