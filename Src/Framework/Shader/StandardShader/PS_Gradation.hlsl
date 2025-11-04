#include "inc_KdStandardShader.hlsli"
#include "../inc_KdCommon.hlsli"

Texture2D g_baseTex : register(t0);
SamplerState g_ss : register(s0);

float4 main(VSOutput In) : SV_Target0
{
    // ベースカラー取得
	float4 baseColor = g_baseTex.Sample(g_ss, In.UV) * g_BaseColor * In.Color;

    // グラデーションの範囲と滑らかさを調整
	float gradientRate = smoothstep(0.02, 4.0, In.UV.x); // 0.2～0.8の範囲で滑らかに
    //float gradientRate = pow(In.UV.y, 1.5); // カーブをつけて境界をぼかす（好みに応じて）

	float3 gradationColor = lerp(baseColor.rgb, g_GradientColor.rgb, gradientRate);

	return float4(gradationColor, baseColor.a);
}
