#include "inc_KdSpriteShader.hlsli"

//============================================
// 2D描画 頂点シェーダ
//============================================
VSOutput main(  float4 pos : POSITION,
			    float2 uv : TEXCOORD0
){
	VSOutput Out;

	// 頂点座標を射影変換
	Out.Pos = mul(pos, g_mTransform);
	Out.Pos = mul(Out.Pos, g_mProj);

	// UV
	Out.UV = uv;

	return Out;
}
