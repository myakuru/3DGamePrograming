// 定数バッファ
cbuffer cbSprite : register(b0)
{
    row_major float4x4  g_mTransform;    // 変換行列
    float4              g_color;
};

cbuffer cbProjection : register(b1)
{
    row_major float4x4  g_mProj;    // 射影変換行列
};

// 頂点シェーダ出力用
struct VSOutput
{
	float4 Pos : SV_Position;
	float2 UV  : TEXCOORD0;
};
