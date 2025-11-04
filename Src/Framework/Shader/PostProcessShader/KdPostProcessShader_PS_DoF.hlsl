#include "../inc_KdCommon.hlsli"
#include "inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex	: register(t0);
Texture2D g_blurTex		: register(t1);
Texture2D g_strongBlurTex : register(t2);
Texture2D g_depthTex	: register(t3);

SamplerState g_ss : register(s0);

cbuffer cb : register(b0)
{
	float g_nearClipDist;
	float g_farClipDist;
	
	float g_focusDistance;
	float g_focusForeRange;
	float g_focusBackRange;
};

// 被写界深度シェーダー
float4 main(VSOutput In) : SV_Target0
{
	float3 color = 0;
	
	float4 depthPixel = g_depthTex.Sample( g_ss, In.UV );
	float2 projXY = (In.UV * 2 - 1) * float2(1, -1);
	depthPixel =  mul(float4(projXY, depthPixel.r, 1), g_mProjInv );
	depthPixel.xyz /= depthPixel.w;
	float depth = saturate((depthPixel.z - g_nearClipDist) / (g_farClipDist - g_nearClipDist));
	
	float focusGap = g_focusDistance - depth;
	float focusRange = focusGap > 0 ? g_focusForeRange : g_focusBackRange;
	
	focusGap = abs(focusGap);
	
	float defaultPow = max( 1.0f - pow( focusGap / focusRange, 2 ), 0.0f );
	float strongBlurPow = saturate( focusGap / focusRange - 1.0f );
	float blurPow = saturate( 1.0f - defaultPow - strongBlurPow );
	
	color += g_inputTex.Sample( g_ss, In.UV ).rgb * defaultPow;
	color += g_blurTex.Sample(g_ss, In.UV).rgb * blurPow;
	color += g_strongBlurTex.Sample(g_ss, In.UV).rgb * strongBlurPow;

	return float4(color, 1);
}
