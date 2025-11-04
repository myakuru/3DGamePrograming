#include "inc_KdSpriteShader.hlsli"

// テクスチャ
Texture2D g_inputTex : register(t0);
// サンプラ
SamplerState g_ss : register(s0);


//============================================
// 2D描画 ピクセルシェーダ
//============================================
float4 main(VSOutput In) : SV_Target0
{
	// テクスチャ色取得
	float4 texCol = g_inputTex.Sample(g_ss, In.UV);
	//アルファテスト
	if (texCol.a < 0.1f) discard;
	
	// テクスチャ色 * 指定色
	return texCol * g_color;
}
