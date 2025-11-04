#include "inc_KdStandardShader.hlsli"
#include "../inc_KdCommon.hlsli"

Texture2D g_baseTex : register(t0);
SamplerState g_ss : register(s0);

float4 main(VSOutput In) : SV_Target0
{
    // ベースカラー取得
	float4 baseColor = g_baseTex.Sample(g_ss, In.UV) * g_BaseColor * In.Color;

    // グレースケール変換（グラデーションなし）
	float gray = dot(baseColor.rgb, float3(0.32f, 0.33f, 0.35f));
	float3 grayColor = float3(gray, gray, gray);

    // カメラへの方向
	float3 vCam = g_CamPos - In.wPos;
	float camDist = length(vCam); // カメラ - ピクセル距離
	vCam = normalize(vCam);

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
			grayColor = lerp(grayColor, g_HeightFogColor, fogRate);
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
		grayColor = lerp(g_DistanceFogColor, grayColor, f);
	}

	return float4(grayColor, baseColor.a);
}
