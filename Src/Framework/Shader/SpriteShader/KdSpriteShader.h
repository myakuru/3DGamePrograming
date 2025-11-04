#pragma once

//===================================================
//
// 2D描画シェーダ
//
//===================================================
class KdSpriteShader {
public:

	struct Vertex
	{
		Math::Vector3 Pos;
		Math::Vector2 UV;
	};

	//===========================================
	// 初期化・解放
	//===========================================

	// 初期化
	bool Init();

	// 解放
	void Release();

	// 
	~KdSpriteShader() {
		Release();
	}

	//===========================================
	//
	// 描画系
	//
	//===========================================

	// 描画開始
	// ステートの記憶・変更や、2D描画用の行列の設定、シェーダ切り替えなどを行う
	// ・linear			… 線形補間モードにする
	// ・disableZBuffer	… Zバッファを使用しない
	void Begin(bool linear = true, bool disableZBuffer = true);

	// 描画終了
	//  Begin()で記憶していたステートを復元 
	void End();

	// 変換行列セット
	void SetMatrix(const Math::Matrix& m)
	{
		m_cb0.Work().mTransform = m;
	}

	// 2D画像描画(Begin～End間で実行すると、処理効率が上がる)
	// ・tex			… 描画するテクスチャ(Texture)
	// ・x				… x座標(ピクセル)
	// ・y				… y座標(ピクセル)
	// ・w				… w座標(ピクセル)
	// ・h				… h座標(ピクセル)
	// ・srcRect		… 元画像のRECT nullptrで全体
	// ・color			… 色(RGBA) nullptrで色はセットしない(前回の描画時の色が使用される)
	// ・pivot			… 基準点 0.0～1.0の範囲で指定する
	void DrawTex(const KdTexture* tex, int x, int y, int w, int h, const Math::Rectangle* srcRect = nullptr, const Math::Color* color = &kWhiteColor, const Math::Vector2& pivot = { 0.5, 0.5f });
	void DrawTex(const std::weak_ptr<KdTexture> tex, int x, int y, int w, int h, const Math::Rectangle* srcRect = nullptr, const Math::Color* color = &kWhiteColor, const Math::Vector2& pivot = { 0.5, 0.5f })
	{
		if(tex.expired())return;
		DrawTex(tex.lock().get(), x, y, w, h, srcRect, color, pivot);
	}

	// 2D画像描画(Begin～End間で実行すると、処理効率が上がる)
	// 幅と高さはtexの情報が使用される
	// ・tex			… 描画するテクスチャ(Texture)
	// ・x				… x座標(ピクセル)
	// ・y				… y座標(ピクセル)
	// ・srcRect		… 元画像のRECT
	// ・color			… 色(RGBA)
	// ・pivot			… 基準点 0.0～1.0の範囲で指定する
	void DrawTex(const KdTexture* tex, int x, int y, const Math::Rectangle* srcRect = nullptr, const Math::Color* color = &kWhiteColor, const Math::Vector2& pivot = { 0.5, 0.5f })
	{
		if (tex == nullptr)return;
		DrawTex(tex, x, y, tex->GetInfo().Width, tex->GetInfo().Height, srcRect, color, pivot);
	}
	void DrawTex(const std::weak_ptr<KdTexture> tex, int x, int y, const Math::Rectangle* srcRect = nullptr, const Math::Color* color = &kWhiteColor, const Math::Vector2& pivot = { 0.5, 0.5f })
	{
		if (tex.expired())return;
		DrawTex(tex.lock().get(), x, y, tex.lock().get()->GetInfo().Width, tex.lock().get()->GetInfo().Height, srcRect, color, pivot);
	}

	// 点を描画
	// ・x				… 点のX座標
	// ・y				… 点のY座標
	// ・color			… 色(RGBA)
	void DrawPoint(int x, int y, const Math::Color* color = &kWhiteColor);

	// 2D線を描画
	// ・x1				… 点１のX座標
	// ・y1				… 点１のY座標
	// ・x2				… 点２のX座標
	// ・y2				… 点２のY座標
	// ・color			… 色(RGBA)
	void DrawLine(int x1, int y1, int x2, int y2, const Math::Color* color = &kWhiteColor);

	// 三角形を描画
	// ・x1				… 座標１のX座標
	// ・y1				… 座標１のY座標
	// ・x2				… 座標２のX座標
	// ・y2				… 座標２のY座標
	// ・x3				… 座標３のX座標
	// ・y3				… 座標３のY座標
	// ・fill			… 塗りつぶし
	void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const Math::Color* color = &kWhiteColor, bool fill = true);

	// 2D円を描画
	// ・x				… 円の中心座標のX座標
	// ・y				… 円の中心座標のY座標
	// ・radius			… 円の半径
	// ・color			… 色(RGBA)
	// ・fill			… 塗りつぶし
	void DrawCircle(int x, int y, int radius, const Math::Color* color = &kWhiteColor, bool fill = true);

	// 2D箱を描画
	// ・x				… 箱の中心座標のX座標
	// ・y				… 箱の中心座標のY座標
	// ・extentX		… 箱のX方向のハーフサイズ
	// ・extentY		… 箱のY方向のハーフサイズ
	// ・color			… 色(RGBA)
	// ・fill			… 塗りつぶし
	void DrawBox(int x, int y, int extentX, int extentY, const Math::Color* color = &kWhiteColor, bool fill = true);

	// 切り抜き範囲を設定する
	// ・rect			… 範囲
	void SetScissorRect(const Math::Rectangle& rect);

private:

	ID3D11VertexShader*		m_VS = nullptr;				// 頂点シェーダー
	ID3D11InputLayout*		m_VLayout = nullptr;		// 頂点レイアウト

	ID3D11PixelShader*		m_PS = nullptr;				// ピクセルシェーダー

	// 定数バッファ
	struct cbSprite {
		Math::Matrix		mTransform;
		Math::Vector4		Color = { 1, 1, 1, 1 };
	};
	KdConstantBuffer<cbSprite>	m_cb0;

	// 定数バッファ
	struct cbProjection {
		Math::Matrix		mProj;
	};
	KdConstantBuffer<cbProjection>	m_cb1;

	// Flag
	bool					m_isBegin = false;

	// 2D用正射影行列
	Math::Matrix			m_mProj2D;
};
