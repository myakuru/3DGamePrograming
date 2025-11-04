#include "inc_KdStandardShader.hlsli"
#include "../inc_KdCommon.hlsli"

Texture2D g_baseTex : register(t0);
SamplerState g_ss : register(s0);

float4 main(VSOutput In) : SV_Target0
{
	if (In.UV.y > g_fadeAmount)
	{
		discard;
	}
	
    // テクスチャカラー取得
	float4 texColor = g_baseTex.Sample(g_ss, In.UV);

    // 明度
	float luminance = dot(texColor.rgb, float3(0.299, 0.587, 0.114));

    // 黒い部分を描画スキップ
	if (texColor.r < 0.1 && texColor.g < 0.1 && texColor.b < 0.1)
	{
		discard;
	}

    // UVの中心からの距離
	float2 center = float2(0.5, 0.5);
	float distance = length(In.UV - center);

    // 距離が遠いほど青、中心ほど白
	float3 iceColor = lerp(float3(g_InnerColor), float3(g_OutColor), saturate(distance * g_colorGradation));

    // 発光強度調整
	float glowPower = pow(luminance, 5.0) * 2.0; // 明度を2乗して強調、さらに2倍

	// 最終色を返すalpfaの値をシェーダー内では透明にならないからcpp側で調整する
	return float4(iceColor * glowPower, g_alphaFade);
}
