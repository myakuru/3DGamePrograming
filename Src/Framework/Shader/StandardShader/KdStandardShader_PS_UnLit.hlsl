#include "inc_KdStandardShader.hlsli"
#include "../inc_KdCommon.hlsli"

// テクスチャ
Texture2D g_tex : register(t0);
Texture2D g_emissiveTex : register(t2); // 発光テクスチャ

Texture2D g_dissolveTex : register(t11); // ディゾルブマップ

// サンプラ
SamplerState g_ss : register(s0);

float4 main(VSOutputNoLighting In) : SV_Target0
{
	// ディゾルブによる描画スキップ
	float discardValue = g_dissolveTex.Sample(g_ss, In.UV).r;
	if (discardValue < g_dissolveValue)
	{
		discard;
	}

	float4 baseColor = g_tex.Sample(g_ss, In.UV) * In.Color * g_BaseColor;
	float3 outColor = baseColor.rgb;
	
	// Alphaテスト
	if (baseColor.a < 0.05f)
	{
		discard;
	}
	
	// 自己発光色の適応
	if (g_OnlyEmissie)
	{
		outColor = g_emissiveTex.Sample(g_ss, In.UV).rgb * g_Emissive * In.Color.rgb;		
	}
	else
	{
		outColor += g_emissiveTex.Sample(g_ss, In.UV).rgb * g_Emissive * In.Color.rgb;
	}
	
	// 全体の明度：環境光に1が設定されている場合は影響なし
	// 環境光の不透明度を下げる事により、明度ライトの周り以外は描画されなくなる
	float totalBrightness = g_AmbientLight.a;

	if (totalBrightness < 1.0f)
	{
		//-------------------------
		// 全体の明度への点光源影響
		//-------------------------
		for (int i = 0; i < g_PointLightNum.x; i++)
		{
		// ピクセルから点光への方向
			float3 dir = g_PointLights[i].Pos - In.wPos;
		
		// 距離を算出
			float dist = length(dir);
		
		// 正規化
			dir /= dist;
		
		// 点光の判定以内
			if (dist < g_PointLights[i].Radius)
			{
			// 半径をもとに、距離の比率を求める
				float atte = 1.0 - saturate(dist / g_PointLights[i].Radius);
			
			// 明度の追加
				totalBrightness += (1 - pow(1 - atte, 2)) * g_PointLights[i].IsBright;
			}
		}
	}
	
	//------------------------------------------
	// 高さフォグ
	//------------------------------------------
	if (g_HeightFogEnable && g_FogEnable)
	{
		if (In.wPos.y < g_HeightFogTopValue)
		{
			float distRate = length(In.wPos - g_CamPos);
			distRate = saturate(distRate / g_HeightFogDistance);
			distRate = pow(distRate, 2.0);
			
			float heightRange = g_HeightFogTopValue - g_HeightFogBottomValue;
			float heightRate = 1 - saturate((In.wPos.y - g_HeightFogBottomValue) / heightRange);
			
			float fogRate = heightRate * distRate;
			outColor = lerp(outColor, g_HeightFogColor, fogRate);
		}
	}
	
	//------------------------------------------
	// 距離フォグ
	//------------------------------------------
	if (g_DistanceFogEnable && g_FogEnable)
	{
		float3 vCam = g_CamPos - In.wPos;
		float camDist = length(vCam); // カメラ - ピクセル距離
		
		// フォグ 1(近い)～0(遠い)
		float f = saturate(1.0 / exp(camDist * g_DistanceFogDensity));
		// 適用
		outColor = lerp(g_DistanceFogColor, outColor, f);
	}
	
	// ディゾルブ輪郭発光
	if (g_dissolveValue > 0)
	{
		// 閾値とマスク値の差分で、縁を検出
		if (abs(discardValue - g_dissolveValue) < g_dissolveEdgeRange)
		{
			// 輪郭に発光色追加
			outColor.rgb += g_dissolveEmissive;
		}
	}
	
	totalBrightness = saturate(totalBrightness);
	outColor *= totalBrightness;
	
	return float4(outColor, baseColor.a);
}
