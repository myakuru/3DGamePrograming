#include "../inc_KdCommon.hlsli"
#include "inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

cbuffer cbNoise : register(b3)
{
	float g_noiseStrength;
	float g_time;
	int g_enableGray; // 灰色化フラグ
	int g_enableNoise;
	int g_warningSign; // 攻撃前兆
	float2 g_targetPos; // 攻撃対象位置
};

// 太い縦ノイズ帯を生成
float verticalBandNoise(float2 uv, float time)
{
    // 画面を128分割（太さ調整：値を変えると太く/細くなる）
	float band = floor(uv.y * 128.0);
    // 各帯ごとに同じノイズ値
	float noise = frac(sin(band + time * 5.0) * 43758.5453) * 2.0 - 1.0;
	return noise * g_noiseStrength * 0.02;
}

float4 main(VSOutput In) : SV_Target0
{
	float2 uv0 = In.UV; // 元UV（位置マーカー用）
	float2 uv = uv0; // サンプリング用UV

    // ノイズ揺らぎはサンプリング側のみ
	if (g_enableNoise == 1)
	{
		uv.x += verticalBandNoise(uv, g_time);
	}

	float3 color = g_inputTex.Sample(g_ss, uv).rgb;

	if (g_enableNoise == 1)
	{
		float n = verticalBandNoise(uv, g_time) * 10.0;
		color = saturate(color + n);
	}

	if (g_enableGray == 1)
	{
		float gray = dot(color, float3(0.299, 0.587, 0.114));
		color = float3(gray, gray, gray);
	}

    // 警告点（未変形UVで固定位置に描く）
	if (g_warningSign == 1)
	{
		float2 center = saturate(g_targetPos.xy);
        // 必要なら上下反転（スクリーンY原点がUVと異なる場合）
        // center.y = 1.0 - center.y;

		float radius = 0.006; // 点の半径（UV空間）
		float aa = 0.002; // アンチエイリアス幅
		float3 dotColor = float3(1.0, 0.1, 0.1);

		float dist = distance(uv0, center);
        // 滑らかな円のアルファ（外側へフェード）
		float alpha = 1.0 - smoothstep(radius - aa, radius + aa, dist);

		color = saturate(lerp(color, dotColor, alpha));
	}

	return float4(color, 1);
}
