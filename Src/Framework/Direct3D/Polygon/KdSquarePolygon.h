#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 四角形ポリゴンクラス
// テクスチャの一部分を描画したり、描画する板ポリのサイズを設定できる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdSquarePolygon : public KdPolygon
{
public:

	// ポリゴンの原点の位置
	enum class PivotType
	{
		Right_Top,		// 右上
		Right_Middle,	// 右中段
		Right_Bottom,	// 右下
		Center_Top = 10,// 中央上
		Center_Middle,	// 中央中段
		Center_Bottom,	// 中央下
		Left_Top = 20,	// 左上
		Left_Middle,	// 左中段
		Left_Bottom,	// 左下
	};

	KdSquarePolygon() { InitVertices(); }
	KdSquarePolygon(const std::shared_ptr<KdTexture>& spBaseColTex) : KdPolygon(spBaseColTex) { InitVertices(); SetScale(1.0f); }
	KdSquarePolygon(const std::string& baseColTexName) : KdPolygon(baseColTexName) { InitVertices(); SetScale(1.0f); }

	~KdSquarePolygon() override {}

	void SetPivot(PivotType pivot);

	// 描画の幅と高さの設定
	void SetScale(const Math::Vector2& scale);
	void SetScale(float _scalar);

	// 頂点の描画色の設定、それぞれの頂点色を指定
	void SetVertexColor(const std::vector<Math::Color>& _vertCols);

	// テクスチャ内の描画エリアの設定
	void SetUVRect(UINT index);
	void SetUVRect(UINT x, UINT y);
	void SetUVRect(const Math::Rectangle& _rect);
	void SetUVRect(const Math::Vector2& _minUV, const Math::Vector2& _maxUV);

	// テクスチャの分割数を設定
	inline void SetSplit(UINT splitX, UINT splitY)
	{
		m_splitX = splitX;
		m_splitY = splitY;
	}

	UINT GetSplitX() { return m_splitX; }
	UINT GetSplitY() { return m_splitY; }

private:

	void InitVertices();

	UINT m_splitX = 1;
	UINT m_splitY = 1;

	PivotType m_pivotType = PivotType::Center_Middle;
};
