struct VSOutput
{
	float4 Pos : SV_Position;
	float2 UV : TEXCOORD0;
	float3 Normal : NORMAL; // 法線
	float4 Color : COLOR0; // 頂点カラー
};
