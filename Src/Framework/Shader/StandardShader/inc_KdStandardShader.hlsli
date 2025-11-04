// 定数バッファ(オブジェクト単位)
cbuffer cbObject : register(b0)
{
	float2	g_UVOffset;
	float2	g_UVTiling;

	int g_ditherEnable; // ディザリング有効
	
	int g_FogEnable;	// フォグ有効
	int g_OnlyEmissie;	// エミッシブの描画だけにするかどうか

	int g_IsSkinMeshObj; // スキンメッシュオブジェクトかどうか(スキンメッシュ対応)
	
	float g_dissolveValue;		// ディゾルブの閾値
	float g_dissolveEdgeRange;	// ディゾルブの境界線の太さ
	float3 g_dissolveEmissive;	// 境界の色

	int g_EnableGradient;
	float4 g_GradientColor;

	int g_LimLightEnable;
	float3 g_LimLightColor;
	float g_LimLightPower;

	float3 g_LitRimLightColor; // リムライトの色
	float g_LitRimLightEnable; // リムライトの有効化フラグ
	float g_LitRimLightPower; // リムライトの強さ
	
};

// 定数バッファ(メッシュ単位)
cbuffer cbMesh : register(b1)
{
	// オブジェクト情報
	row_major float4x4 g_mWorld; // ワールド変換行列
};

cbuffer cbMaterial : register(b2)
{
	float4	g_BaseColor; // ベース色
	float3	g_Emissive;  // 自己発光色
	float	g_Metallic;	 // 金属度
	float	g_Roughness; // 粗さ
};

// ボーン行列配列(スキンメッシュ対応)
cbuffer cbBones : register(b3)
{
	row_major float4x4 g_mBones[500];
};

cbuffer cbEffect : register(b4)
{
	float g_fadeAmount; // フェードアウト量（0.0～1.0、外部で調整可能に）
	float3 g_OutColor; // フェードアウト時の色
	float3 g_InnerColor; // フェードアウト時の中心の色
	float g_colorGradation; // 色のグラデーション量
	float g_alphaFade; // アルファフェード量
};

// 頂点シェーダから出力するデータ
struct VSOutput
{
					// :　hogehoge  これはセマンティクスで、頂点シェーダからピクセルシェーダに渡すデータの名前を指定します。
	float4 Pos	 : SV_Position;	// 射影座標
	float3 wPos  : TEXCOORD0;	// ワールド3D座標

	float2 UV	 : TEXCOORD1;	// UV座標
	float4 Color : TEXCOORD2;	// 色

	float3 wN	 : TEXCOORD3;	// ワールド法線
	float3 wT	 : TEXCOORD4;	// ワールド接線
	float3 wB	 : TEXCOORD5;	// ワールド従法線
};

struct VSOutputNoLighting
{
	float4 Pos	 : SV_Position; // 射影座標
	float3 wPos  : TEXCOORD0;	// ワールド3D座標

	float2 UV	 : TEXCOORD1;	// UV座標
	float4 Color : TEXCOORD2;	// 色
};

struct VSOutputGenShadow
{
	float4 Pos	 : SV_Position;	// 射影座標
	float4 pPos  : TEXCOORD0;	// 射影座標（VP変換無し

	float2 UV	 : TEXCOORD1;	// UV座標
	float4 Color : TEXCOORD2;	// 色
};
