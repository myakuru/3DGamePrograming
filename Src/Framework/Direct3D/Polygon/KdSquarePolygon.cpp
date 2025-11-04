#include "KdSquarePolygon.h" 


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 板ポリゴンの原点の設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdSquarePolygon::SetPivot(PivotType pivot)
{
	m_pivotType = pivot;

	Math::Vector2 scale;
	scale.x = m_vertices[3].pos.x - m_vertices[0].pos.x;
	scale.y = m_vertices[3].pos.y - m_vertices[0].pos.y;

	SetScale(scale);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画の幅と高さの設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdSquarePolygon::SetScale(const Math::Vector2& scale)
{
	float halfX = scale.x * 0.5f;
	float halfY = scale.y * 0.5f;

	float pivotX = (static_cast<int>(m_pivotType) / 10 - 1) * halfX;
	float pivotY = (static_cast<int>(m_pivotType) % 10 - 1) * halfY;

	// 頂点座標
	m_vertices[0].pos = { -halfX + pivotX, -halfY + pivotY, 0.0f };	// 左上
	m_vertices[1].pos = { -halfX + pivotX,  halfY + pivotY, 0.0f };	// 左下
	m_vertices[2].pos = { halfX + pivotX, -halfY + pivotY, 0.0f };	// 右上
	m_vertices[3].pos = { halfX + pivotX,  halfY + pivotY, 0.0f };	// 右下
}

void KdSquarePolygon::SetScale(float scalar)
{
	Math::Vector2 scale;

	scale.x = m_vertices[3].pos.x - m_vertices[0].pos.x;
	scale.y = m_vertices[3].pos.y - m_vertices[0].pos.y;

	// アスペクト比を保ったまま拡縮する、
	if (scale.x == 0.0f || scale.y == 0.0f)
	{
		// どちらかのscaleが0だった場合の0除算対策
		scale.Normalize();
	}
	// 短い方を長さ1.0に正規化
	else if (scale.x > scale.y)
	{
		scale /= scale.y;
	}
	else
	{
		scale /= scale.x;
	}

	scale *= scalar;

	SetScale(scale);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// テクスチャの描画合成色の設定
// それぞれの頂点で色を変えたい特殊な状況でのみ使用する関数：全体を変えたいときはSetColor
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdSquarePolygon::SetVertexColor(const std::vector<Math::Color>& _vertCols)
{
	size_t colNum = _vertCols.size();

	if(!colNum)
	{
		assert(0 && "KdSquarePolygon::SetVertexColor 色が一色も指定されていません");
	}

	// 送られた色数分頂点色を書き換える
	for (size_t i = 0; i < colNum; ++i)
	{
		// 送られてきた色の数が板ポリの頂点数4つを越えた場合も終了
		if (i >= m_vertices.size()) { return; }

		unsigned int col = 0;
		unsigned char r = static_cast<unsigned char>(_vertCols[i].R() * 255);
		unsigned char g = static_cast<unsigned char>(_vertCols[i].G() * 255);
		unsigned char b = static_cast<unsigned char>(_vertCols[i].B() * 255);
		unsigned char a = static_cast<unsigned char>(_vertCols[i].A() * 255);

		col = (a << 24) | (b << 16) | (g << 8) | r;

		m_vertices[i].color = col;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// テクスチャ内の描画エリアの設定 splitX,Yを使って
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdSquarePolygon::SetUVRect(UINT index)
{
	// マス座標
	int x = index % m_splitX;
	int y = index / m_splitX;

	SetUVRect(x, y);
}

void KdSquarePolygon::SetUVRect(UINT x, UINT y)
{
	float w = 1.0f / m_splitX;
	float h = 1.0f / m_splitY;

	Math::Vector2 uvMin, uvMax;

	uvMin.x = x * w;
	uvMin.y = y * h;

	uvMax.x = uvMin.x + w;
	uvMax.y = uvMin.y + h;

	SetUVRect(uvMin, uvMax);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// テクスチャ内の描画エリアの設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdSquarePolygon::SetUVRect(const Math::Rectangle& _rect)
{
	if (!m_spMaterial) { return; }

	Math::Vector2 uvMin, uvMax;
	ConvertRectToUV(m_spMaterial->m_baseColorTex.get(), _rect, uvMin, uvMax);

	// UV座標
	SetUVRect(uvMin, uvMax);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdSquarePolygon::SetUVRect(const Math::Vector2& _uvMin, const Math::Vector2& _uvMax)
{
	// UV座標
	m_vertices[0].UV = { _uvMin.x, _uvMax.y };
	m_vertices[1].UV = { _uvMin.x, _uvMin.y };
	m_vertices[2].UV = { _uvMax.x, _uvMax.y };
	m_vertices[3].UV = { _uvMax.x, _uvMin.y };
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// スケール：1mの正方形
// UV：
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdSquarePolygon::InitVertices()
{
	m_vertices.resize(4);

	// ローカル頂点座標縦横 1m の板ポリゴン
	m_vertices[0].pos = { -0.5f, -0.5f, 0 };
	m_vertices[1].pos = { -0.5f,  0.5f, 0 };
	m_vertices[2].pos = {  0.5f, -0.5f, 0 };
	m_vertices[3].pos = {  0.5f,  0.5f, 0 };

	// UV座標
	m_vertices[0].UV = { 0, 1 };	// 左上
	m_vertices[1].UV = { 0, 0 };	// 左下
	m_vertices[2].UV = { 1, 1 };	// 右上
	m_vertices[3].UV = { 1, 0 };	// 右下

	// とりあえず板ポリは2Dオブジェクトとして生成する
	Set2DObject(true);
}
