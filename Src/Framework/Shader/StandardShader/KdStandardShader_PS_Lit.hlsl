#include "inc_KdStandardShader.hlsli"
#include "../inc_KdCommon.hlsli"

// モデル描画用テクスチャ
Texture2D g_baseTex : register(t0); // ベースカラーテクスチャ
Texture2D g_metalRoughTex : register(t1); // メタリック/ラフネステクスチャ
Texture2D g_emissiveTex : register(t2); // 発光テクスチャ
Texture2D g_normalTex : register(t3); // 法線マップ

// 特殊処理用テクスチャ
Texture2D g_dirShadowMap : register(t10); // 平行光シャドウマップ
Texture2D g_dissolveTex : register(t11); // ディゾルブマップ
Texture2D g_environmentTex : register(t12); // 反射景マップ

// サンプラ
SamplerState g_ss : register(s0); // 通常のテクスチャ描画用
SamplerComparisonState g_ssCmp : register(s1); // 補間用比較機能付き

static const float PI = 3.1415926535;

// 旧Blinn-Phong（未使用化）
float BlinnPhong(float3 lightDir, float3 vCam, float3 normal, float specPower)
{
	float3 H = normalize(-lightDir + vCam);
	float NdotH = saturate(dot(normal, H)); // カメラの角度差(0～1)
	float spec = pow(NdotH, specPower);

	// 正規化Blinn-Phong
	return spec * ((specPower + 2) / (2 * 3.1415926535));
}

//===============================
// PBR(Disney + Cook-Torrance) ヘルパ
//===============================
float Pow5(float x)
{
	// 高速化された (1-x)^5 にも利用
	float x2 = x * x;
	return x2 * x2 * x;
}

float DistributionGGX(float NdotH, float alpha)
{
	// Trowbridge-Reitz GGX
	float a2 = alpha * alpha;
	float d = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
	return a2 / max(PI * d * d, 1e-7);
}

float GeometrySchlickGGX(float NdotX, float k)
{
	// Schlick-GGX(直接照明用)
	return NdotX / (NdotX * (1.0 - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	// Smithに基づく分離近似
	// UE4推奨のk=(r+1)^2/8
	float r = max(roughness, 1e-4);
	float k = (r + 1.0) * (r + 1.0) * 0.125;
	float ggxV = GeometrySchlickGGX(NdotV, k);
	float ggxL = GeometrySchlickGGX(NdotL, k);
	return ggxV * ggxL;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	// Schlick近似
	return F0 + (1.0 - F0) * Pow5(1.0 - cosTheta);
}

float3 DisneyDiffuse(float3 albedo, float NdotV, float NdotL, float LdotH, float roughness)
{
	// Disney 2012 Diffuse
	float FD90 = 0.5 + 2.0 * LdotH * LdotH * roughness;
	float FL = 1.0 + (FD90 - 1.0) * Pow5(1.0 - NdotL);
	float FV = 1.0 + (FD90 - 1.0) * Pow5(1.0 - NdotV);
	return (albedo / PI) * (FL * FV);
}

//================================
// ピクセルシェーダ
//================================
float4 main(VSOutput In) : SV_Target0
{
	// ディゾルブによる描画スキップ
	float discardValue = g_dissolveTex.Sample(g_ss, In.UV).r;
	if (discardValue < g_dissolveValue)
	{
		discard;
	}
	
	//------------------------------------------
	// 材質色
	//------------------------------------------
	float4 baseColor = g_baseTex.Sample(g_ss, In.UV) * g_BaseColor * In.Color;
	
	// Alphaテスト
	if (baseColor.a < 0.05f)
	{
		discard;
	}
	
	// カメラへの方向
	float3 vCam = g_CamPos - In.wPos;
	float camDist = length(vCam); // カメラ - ピクセル距離
	vCam = normalize(vCam);

	// 法線マップから法線ベクトル取得
	float3 wN = g_normalTex.Sample(g_ss, In.UV).rgb;

	// UV座標（0～1）から 射影座標（-1～1）へ変換
	wN = wN * 2.0 - 1.0;

	// 法線マップをリターンする
	//return float4(normalize(wN), 1.0);
	
	{
		// 3種の法線から法線行列を作成
		row_major float3x3 mTBN =
		{
			normalize(In.wT),
			normalize(In.wB),
			normalize(In.wN),
		};
	
		// 法線ベクトルをこのピクセル空間へ変換
		wN = mul(wN, mTBN);
	}

	// 法線正規化
	wN = normalize(wN);

	float4 mr = g_metalRoughTex.Sample(g_ss, In.UV);
	// 金属性
	float metallic = saturate(mr.b * g_Metallic);
	// 粗さ
	float roughness = saturate(mr.g * g_Roughness);
	// アンビエントオクルージョン
	float ao = saturate(mr.r);

	ao = max(ao, 0.2);
	
	// GGXのα(マイクロファセットの粗さ)は roughness^2 が一般的
	float alpha = max(roughness * roughness, 1e-4);
	
	// Fresnelのベース反射率(F0): 非金属は0.04、金属はベースカラー
	float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor.rgb, metallic);

	//------------------------------------------
	// ライティング
	//------------------------------------------
	float3 outColor = 0;
	
	//-------------------------------
	// シャドウマッピング(影判定) - 平行光用
	//-------------------------------
	float shadow = 1;

	// ピクセルの3D座標から、DepthMapFromLight空間へ変換
	float4 liPos = mul(float4(In.wPos, 1), g_DL_mLightVP);
	liPos.xyz /= liPos.w;

	// 深度マップの範囲内？
	if (abs(liPos.x) <= 1 && abs(liPos.y) <= 1 && liPos.z <= 1)
	{
		// 射影座標 -> UV座標へ変換
		float2 uv = liPos.xy * float2(1, -1) * 0.5 + 0.5;
		// ライトカメラからの距離
		float z = liPos.z - 0.004; // シャドウアクネ対策
		
		// 画像のサイズからテクセルサイズを求める
		float w, h;
		g_dirShadowMap.GetDimensions(w, h);
		float tw = 1.0 / w;
		float th = 1.0 / h;
	
		// uvの周辺3x3も判定し、平均値を求める
		shadow = 0;
		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{
				shadow += g_dirShadowMap.SampleCmpLevelZero(g_ssCmp, uv + float2(x * tw, y * th), z);
			}
		}
		shadow *= 0.11;
	}
		
	//-------------------------
	// 平行光 (Disney + Cook-Torrance)
	//-------------------------
	{
		float3 L = normalize(-g_DL_Dir);
		float3 V = normalize(vCam);
		float3 H = normalize(L + V);

		float NdotL = saturate(dot(wN, L));
		float NdotV = saturate(dot(wN, V));
		float NdotH = saturate(dot(wN, H));
		float LdotH = saturate(dot(L, H));

		// BRDF 構成要素
		float D = DistributionGGX(NdotH, alpha);
		float G = GeometrySmith(NdotV, NdotL, roughness);
		float3 F = FresnelSchlick(saturate(dot(H, V)), F0);
		float3 specBRDF = (D * G) * F / max(4.0 * NdotV * NdotL, 1e-4);

		// エネルギー保存: kS=F, kD=(1-kS)*(1-metallic)
		float3 kS = F;
		float3 kD = (1.0 - kS) * (1.0 - metallic);

		// Disney Diffuse
		float3 diffuseBRDF = DisneyDiffuse(baseColor.rgb, NdotV, NdotL, LdotH, roughness);

		// 放射照度項 (ライト色) を掛け、NdotLで重み付け
		float3 lightColor = g_DL_Color;
		float3 contrib = (kD * diffuseBRDF + specBRDF) * lightColor * NdotL;

		// 透明度とシャドウ
		outColor += contrib * baseColor.a * shadow;
	}

	// 全体の明度：環境光に1が設定されている場合は影響なし
	float totalBrightness = g_AmbientLight.a;

	//-------------------------
	// 点光 (Disney + Cook-Torrance)
	//-------------------------
	for (int i = 0; i < g_PointLightNum.x; i++)
	{
		// ピクセルから点光への方向
		float3 L = g_PointLights[i].Pos - In.wPos;
		
		// 距離を算出
		float dist = length(L);
		
		// 正規化
		L /= dist;
		
		// 点光の判定以内
		if (dist < g_PointLights[i].Radius)
		{
			// 半径をもとに、距離の比率を求める
			float atte = 1.0 - saturate(dist / g_PointLights[i].Radius);
			
			// 明度の追加
			totalBrightness += (1 - pow(1 - atte, 2)) * g_PointLights[i].IsBright;
			
			// 逆２乗の法則
			atte *= atte;

			float3 V = normalize(vCam);
			float3 H = normalize(L + V);

			float NdotL = saturate(dot(wN, L));
			float NdotV = saturate(dot(wN, V));
			float NdotH = saturate(dot(wN, H));
			float LdotH = saturate(dot(L, H));

			float D = DistributionGGX(NdotH, alpha);
			float G = GeometrySmith(NdotV, NdotL, roughness);
			float3 F = FresnelSchlick(saturate(dot(H, V)), F0);
			float3 specBRDF = (D * G) * F / max(4.0 * NdotV * NdotL, 1e-4);

			float3 kS = F;
			float3 kD = (1.0 - kS) * (1.0 - metallic);

			float3 diffuseBRDF = DisneyDiffuse(baseColor.rgb, NdotV, NdotL, LdotH, roughness);

			float3 lightColor = g_PointLights[i].Color * atte;
			float3 contrib = (kD * diffuseBRDF + specBRDF) * lightColor * NdotL;

			outColor += contrib * baseColor.a;
		}
	}

	// 環境光（簡易IBLの代替・拡散のみ、金属は拡散を抑制）
	outColor += g_AmbientLight.rgb * baseColor.rgb * (1.0 - metallic) * baseColor.a * ao;
	
	// 自己発光色の適応
	if (g_OnlyEmissie)
	{
		outColor = g_emissiveTex.Sample(g_ss, In.UV).rgb * g_Emissive * In.Color.rgb;
	}
	else
	{
		outColor += g_emissiveTex.Sample(g_ss, In.UV).rgb * g_Emissive * In.Color.rgb;
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
			outColor += g_dissolveEmissive;
		}
	}
	
	totalBrightness = saturate(totalBrightness);
	outColor *= totalBrightness;

	 // ビュー方向
	float3 V = normalize(g_CamPos - In.wPos);

	// リムライト

	if (g_LitRimLightEnable)
	{
		float NdotV = saturate(dot(wN, V));
		float rim = pow(1.0 - NdotV, 5.0);
		outColor += g_LitRimLightColor * rim * g_LitRimLightPower;
	}

	//------------------------------------------
	// 出力
	//------------------------------------------
	return float4(outColor, baseColor.a);
}
