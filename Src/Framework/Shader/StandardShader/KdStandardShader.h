#pragma once
//============================================================
//
// 基本シェーダー
//
//============================================================
class KdStandardShader
{
public:

	// スキンメッシュ対応
	static const int maxBoneBufferSize = 500;

	// 定数バッファ(オブジェクト単位更新)
	struct cbObject
	{
		// UV操作
		Math::Vector2	UVOffset = { 0.0f, 0.0f };
		Math::Vector2	UVTiling = { 1.0f, 1.0f };

		// ディザリング有効
		int             ditherEnable = 1;

		// フォグ有効
		int				FogEnable = 1;

		// エミッシブのみの描画
		int				OnlyEmissie = 0;

		// スキンメッシュオブジェクトかどうか(スキンメッシュ対応)
		int				IsSkinMeshObj = 0;

		// ディゾルブ関連（float2 の後に float3 が来るため、HLSL と同じ16B境界に揃える）
		float			DissolveThreshold = 0.0f;	// 0 ～ 1
		float			DissolveEdgeRange = 0.03f;	// 0 ～ 1
		float			_pad0[2] = { 0.0f, 0.0f };   // ← ここで8バイト詰め物（HLSLのレジスタ境界に揃える）

		Math::Vector3	DissolveEmissive = { 0.0f, 1.0f, 1.0f };

		// グラデーション用フラグとカラー
		int             enableGradient = 0;           // 0:無効, 1:有効
		Math::Color     gradientColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // float4

		// リムライト
		int             LimLightEnable = 0;                  // int + float3 で同一レジスタに収まる
		Math::Vector3   LimLightColor = { 1.0f, 1.0f, 1.0f };
		float           LimLightPower = 1.0f;

		Math::Vector3   LitRimLightColor = { 1.0f, 1.0f, 1.0f };
		float           LitRimLightEnable = 0.0f;
		float			LitRimLightPow = 4.0f;

		float			_pad1[2] = { 0.0f, 0.0f };   // ← ここで8バイト詰め物（HLSLのレジスタ境界に揃える）

	};

	// 定数バッファ(メッシュ単位更新)
	struct cbMesh
	{
		Math::Matrix	mW;
	};

	// 定数バッファ(マテリアル単位更新)
	struct cbMaterial
	{
		Math::Vector4	BaseColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		Math::Vector3	Emissive = { 1.0f, 1.0f, 1.0f };
		float			Metallic = 0.0f;

		float			Roughness = 1.0f;
		float			_blank[3] = { 0.0f, 0.0f ,0.0f };
	};

	// 定数バッファ(ボーン単位更新：スキンメッシュ対応)
	struct cbBone {
		Math::Matrix	mBones[500];
	};

	// エフェクト用
	struct cbEffect
	{
		float fadeAmount = 0.0f;
		Math::Vector3 OutColor = { 0.2f, 0.7f, 1.0f };
		Math::Vector3 InColor = { 1.0f, 1.0f, 1.0f };
		float colorGradation = 5.0f;
		float alphaFade = 1.0f; // 透明化の度合い(1.0で完全透明)
		float _padding[3] = { 0.0f, 0.0f, 0.0f };
	};

	//================================================
	// 設定・取得
	//================================================

	// UVタイリング設定
	void SetUVTiling(const Math::Vector2& tiling)
	{
		m_cb0_Obj.Work().UVTiling = tiling;

		m_dirtyCBObj = true;
	}

	// UVオフセット設定
	void SetUVOffset(const Math::Vector2& offset)
	{
		m_cb0_Obj.Work().UVOffset = offset;

		m_dirtyCBObj = true;
	}

	// フォグ有効/無効
	void SetFogEnable(bool enable)
	{
		m_cb0_Obj.Work().FogEnable = enable;

		m_dirtyCBObj = true;
	}

	// ディゾルブ設定
	void SetDissolve(float threshold, const float* range = nullptr, const Math::Vector3* edgeColor = nullptr)
	{
		auto& cbObj = m_cb0_Obj.Work();

		cbObj.DissolveThreshold = threshold;

		if (range)
		{
			cbObj.DissolveEdgeRange = *range;
		}

		if (edgeColor)
		{
			cbObj.DissolveEmissive = *edgeColor;
		}

		m_dirtyCBObj = true;
	}

	// ディゾルブテクスチャ設定
	void SetDissolveTexture(KdTexture& dissolveMask)
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(11, 1, dissolveMask.WorkSRViewAddress());
	}

	// デフォルトディゾルブテクスチャ設定
	void SetDefaultDissolveTexture(std::shared_ptr<KdTexture>& spDissolveMask)
	{
		if (!spDissolveMask) { return; }

		m_dissolveTex = spDissolveMask;

		SetDissolveTexture(*spDissolveMask);
	}

	// デフォルトのディゾルブテクスチャに戻す
	void ResetDissolveTexture()
	{
		if (!m_dissolveTex) { return; }

		SetDissolveTexture(*m_dissolveTex);
	}

	// アルファディザリング有効/無効
	void SetDitherEnable(bool dither)
	{
		m_cb0_Obj.Work().ditherEnable = dither;
		m_dirtyCBObj = true;
	}

	// リムライトの設定
	void SetRimLightEnable(bool enable)
	{
		m_cb0_Obj.Work().LimLightEnable = enable ? 1 : 0;
		m_dirtyCBObj = true;
	}

	void SetRimLight(float pow, const Math::Vector3& color)
	{
		m_cb0_Obj.Work().LimLightPower = pow;
		m_cb0_Obj.Work().LimLightColor = color;
		m_dirtyCBObj = true;
	}

	// DrawLit用リムライト設定
	void SetLitRimLight(const Math::Vector3& _color = { 1.0f,1.0f,1.0f }, bool _enable = false, float _ritRimLightPower = 1.0f)
	{
		m_cb0_Obj.Work().LitRimLightColor = _color;
		m_cb0_Obj.Work().LitRimLightEnable = _enable ? 1.0f : 0.0f;
		m_cb0_Obj.Work().LitRimLightPow = _ritRimLightPower;
		m_dirtyCBObj = true;
	}

	//================================================
	// 各定数バッファの取得
	//================================================
	const cbObject& GetObjectCB() const { return m_cb0_Obj.Get(); }

	const cbMesh& MeshCB() const { return m_cb1_Mesh.Get(); }

	const cbMaterial& WorkMaterialCB() const { return m_cb2_Material.Get(); }

	// スキンメッシュ対応
	const cbBone& WorkBoneCB() const { return m_cb3_Bone.Get(); }

	//================================================
	// 描画準備
	//================================================
	// 陰影をつけるオブジェクト等を描画する前後に行う
	void BeginLit();
	void EndLit();

	// 陰影をつけないオブジェクト等を描画する前後に行う
	void BeginUnLit();
	void EndUnLit();

	// 最も初めに行う、光を遮るオブジェクトを描画する前後に行う
	void BeginGenerateDepthMapFromLight();
	void EndGenerateDepthMapFromLight();

	// トゥーンシェーダーを使用する前後に行う
	void BeginToon();
	void EndToon();

	// グラデーションを有効にする
	void BeginGradient();
	void EndGradient();

	// グレースケール
	void BeginGrayscale();
	void EndGrayscale();

	// エフェクト用
	void BeginEffect();
	void EndEffect();

	// RimLight用描画開始・終了
	void BeginRimLight();
	void EndRimLight();

	//================================================
	// 描画関数
	//================================================
	// メッシュ描画
	void DrawMesh(const KdMesh* mesh, const Math::Matrix& mWorld, const std::vector<KdMaterial>& materials,
		const Math::Vector4& col, const Math::Vector3& emissive);

	// モデルデータ描画：アニメーションに非対応
	void DrawModel(const KdModelData& rModel, const Math::Matrix& mWorld = Math::Matrix::Identity, 
		const Math::Color& colRate = kWhiteColor, const Math::Vector3& emissive = Math::Vector3::Zero);

	// モデルワーク描画：アニメーションに対応
	void DrawModel(KdModelWork& rModel, const Math::Matrix& mWorld = Math::Matrix::Identity,
		const Math::Color& colRate = kWhiteColor, const Math::Vector3& emissive = Math::Vector3::Zero);

	// 任意の頂点群からなるポリゴン描画
	void DrawPolygon(const KdPolygon& poly, const Math::Matrix& mWorld = Math::Matrix::Identity,
		const Math::Color& colRate = kWhiteColor, const Math::Vector3& emissive = Math::Vector3::Zero);

	// 任意の頂点群からなるポリゴンライン描画
	void DrawVertices(const std::vector<KdPolygon::Vertex>& vertices, const Math::Matrix& mWorld = Math::Matrix::Identity,
		const Math::Color& colRate = kWhiteColor);

	//================================================
	// 初期化・解放
	//================================================

	// 初期化
	bool Init();
	// 解放
	void Release();

	~KdStandardShader()
	{
		Release();
	}

	std::shared_ptr<KdTexture>& GetDepthTex() { return m_depthMapFromLightRTPack.m_RTTexture; }

	// 自分で追加
	bool IsEnableGradient() const { return m_enableGradient; }
	void SetEnableGradient(bool enable)
	{ 
		m_enableGradient = enable; 
		m_cb0_Obj.Work().enableGradient = enable ? 1 : 0; // 0:無効, 1:有効
		m_dirtyCBObj = true; 
	}
	void SetGradientColor(const Math::Color& color)
	{
		m_gradientColor = color;
	}

	// フェード量の設定(エフェクト用)
	void SetFadeAmount(float _fade, Math::Vector3 _outColor = { 0.2f,0.7f,1.0f }, Math::Vector3 _inColor = { 1,1,1 },float _colorGradation = 5.0f, float _alphaFade = 1.0f)
	{
		m_cb4_Effect.Work().fadeAmount = _fade;		// 基本０から１の間で描画するかしないか決める
		m_cb4_Effect.Work().OutColor = _outColor;	// オブジェクトの外側の色
		m_cb4_Effect.Work().InColor = _inColor;		// オブジェクトの内側の色
		m_cb4_Effect.Work().colorGradation = _colorGradation; // 色のグラデーションの度合い
		m_cb4_Effect.Work().alphaFade = _alphaFade; // 透明化の度合い(1.0で完全透明)
		m_cb4_Effect.Write();
		m_dirtyCBObj = true;
	}

private:

	// マテリアルのセット
	void WriteMaterial(const KdMaterial& material, const Math::Vector4& colRate, const Math::Vector3& emiRate);

	// ポリゴンの法線情報を2Dように書き換える
	void ConvertNormalsFor2D(std::vector<KdPolygon::Vertex>& target, const Math::Matrix& mWorld);

	// 定数バッファを初期状態に戻す
	void ResetCBObject();

	// スキンメッシュ有効かどうか(スキンメッシュ対応)
	void SetIsSkinMeshObj(bool isSkinMeshObj)
	{
		if (m_cb0_Obj.Work().IsSkinMeshObj != isSkinMeshObj)
		{
			m_cb0_Obj.Work().IsSkinMeshObj = isSkinMeshObj;

			m_dirtyCBObj = true;
		}
	}

	// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
	// Lit：陰影をつけるオブジェクトの描画用（不透明な物体やキャラクタの板ポリなど
	// 平行光・点光源などの影響を受け角度によって色を変化させるオブジェクトを描画するシェーダー
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// UnLit：陰影のつかないオブジェクトの描画用（エフェクトや透明な物体など
	// 光の計算を行わずマテリアルの色をそのまま出力するシェーダー
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// GenDepthFromLight：光から見たオブジェクトの距離を赤で出力用
	// Litシェーダーで影の描画を行うために必要な情報テクスチャを作成するシェーダー
	// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////

	// 頂点シェーダー
	ID3D11VertexShader* m_VS_Lit = nullptr;					// 陰影あり
	ID3D11VertexShader* m_VS_UnLit = nullptr;				// 陰影なし
	ID3D11VertexShader* m_VS_GenDepthFromLight = nullptr;	// 光からの深度

	// 頂点入力レイアウト
	ID3D11InputLayout* m_inputLayout = nullptr;
	
	// ピクセルシェーダー
	ID3D11PixelShader* m_PS_Lit = nullptr;					// 陰影あり
	ID3D11PixelShader* m_PS_UnLit = nullptr;				// 陰影なし
	ID3D11PixelShader* m_PS_GenDepthFromLight = nullptr;	// 光からの深度

	ID3D11PixelShader* m_PS_Toon = nullptr;					// トゥーン用ピクセルシェーダー

	ID3D11PixelShader* m_PS_Gradation = nullptr;				// グラーデーション用ピクセルシェーダー
	ID3D11PixelShader* m_PS_GrayScale = nullptr;				// グレースケール用ピクセルシェーダー
	ID3D11PixelShader* m_PS_Effect = nullptr;					// エフェクト用ピクセルシェーダー
	ID3D11PixelShader* m_PS_RimLight = nullptr;				// リムライト用ピクセルシェーダー

	// テクスチャ
	std::shared_ptr<KdTexture>	m_dissolveTex = nullptr;	// ディゾルブで使用するデフォルトテクスチャ

	// 定数バッファ
	KdConstantBuffer<cbObject>		m_cb0_Obj;				// オブジェクト単位で更新
	KdConstantBuffer<cbMesh>		m_cb1_Mesh;				// メッシュ毎に更新
	KdConstantBuffer<cbMaterial>	m_cb2_Material;			// マテリアル毎に更新
	KdConstantBuffer<cbBone>		m_cb3_Bone;				// ボーン毎に更新(スキンメッシュ対応)
	KdConstantBuffer<cbEffect >		m_cb4_Effect;			// エフェクト用

	KdRenderTargetPack	m_depthMapFromLightRTPack;
	KdRenderTargetChanger m_depthMapFromLightRTChanger;

	bool		m_dirtyCBObj = false;						// 定数バッファのオブジェクトに変更があったかどうか


	// 自分で追加
	//　グラデーション用フラグとカラー
	bool m_enableGradient = false;
	Math::Color m_gradientColor{1.0f,1.0f,1.0f,1.0f};
	float m_fadeAmount = 1.0f;

	float m_time = 0.0f;
};
