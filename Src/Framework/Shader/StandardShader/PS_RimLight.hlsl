#include "inc_KdStandardShader.hlsli"
#include "../inc_KdCommon.hlsli"

// 必要最小限のリソースだけ再宣言（レジスタは本体と合わせる）
Texture2D g_baseTex : register(t0);
Texture2D g_normalTex : register(t3);
Texture2D g_dissolveTex : register(t11);

SamplerState g_ss : register(s0);

float4 main(VSOutput In) : SV_Target0
{

    // 法線取得（TBN 変換）
	float3 wN = g_normalTex.Sample(g_ss, In.UV).rgb * 2.0 - 1.0;
	row_major float3x3 mTBN =
	{
		normalize(In.wT),
        normalize(In.wB),
        normalize(In.wN),
	};
	wN = normalize(mul(wN, mTBN));

    // ビュー方向
	float3 V = normalize(g_CamPos - In.wPos);

    // リム（Fresnel ライク）
	float NdotV = saturate(dot(wN, V));
	float rim = pow(1.0 - NdotV, 5.0);

    // リムライト「色だけ」を出力
	float3 rimColor = g_LimLightEnable ? (g_LimLightColor * rim * g_LimLightPower) : 0;

	return float4(rimColor, 1.0);
}
