#include "KdTrailPolygon.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 設定されたパターンに従って帯状ポリゴンを生成する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdTrailPolygon::GenerateVertices()
{
	switch (m_pattern)
	{
		case Trail_Pattern::eDefault:		CreateVerticesWithDefaultPattern();		break;
		case Trail_Pattern::eBillboard:		CreateVerticesWithBillboardPattern();	break;
		case Trail_Pattern::eVertices:		CreateVerticesWithVerticesPattern();	break;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 行列情報のまま帯状ポリゴンを生成
// X軸の拡大率で帯の幅を調整可能
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdTrailPolygon::CreateVerticesWithDefaultPattern()
{
	m_vertices.clear();

	// ポイントが２つ以下の場合はポリゴン生成不可
	if (m_pointList.size() < 2) { return; }

	// 軌跡画像の分割数
	float sliceCount = (float)(m_pointList.size() - 1);

	// ポイント数分確保
	m_vertices.resize(m_pointList.size() * 2);

	//--------------------------
	// 頂点データ作成
	//--------------------------
	for (UINT i = 0; i < m_pointList.size(); i++)
	{
		// 行列取得
		const Math::Matrix& mat = m_pointList[i];

		// ２つの頂点の参照(ショートカット)
		Vertex& v1 = m_vertices[i * 2];
		Vertex& v2 = m_vertices[i * 2 + 1];

		// X方向
		Math::Vector3 axisX = mat.Right();
		float width = axisX.Length() * 0.5f;

		axisX.Normalize();

		// 座標
		v1.pos = mat.Translation() + axisX * width * 0.5f;
		v2.pos = mat.Translation() - axisX * width * 0.5f;

		// UV
		float uvY = i / float(sliceCount);

		v1.UV = { 0, uvY };
		v2.UV = { 1, uvY };
	}
}

void KdTrailPolygon::CreateVerticesWithBillboardPattern()
{
	m_vertices.clear();

	// ポイントが２つ以下の場合は描画不可
	if (m_pointList.size() < 2) { return; }

	// カメラの情報
	Math::Matrix mCam = KdShaderManager::Instance().GetCameraCB().mView.Invert();

	// 軌跡画像の分割数
	float sliceCount = (float)(m_pointList.size() - 1);

	// ポイント数分確保
	m_vertices.resize(m_pointList.size() * 2);

	//--------------------------
	// 頂点データ作成
	//--------------------------
	Math::Vector3 prevPos;
	for (UINT i = 0; i < m_pointList.size(); i++)
	{
		// 行列取得
		const Math::Matrix& mat = m_pointList[i];

		// ２つの頂点の参照(ショートカット)
		Vertex& v1 = m_vertices[i * 2];
		Vertex& v2 = m_vertices[i * 2 + 1];

		// ラインの向き
		Math::Vector3 vDir;
		if (i == 0)
		{
			// 初回時のみ、次のポイントを使用
			vDir = m_pointList[1].Translation() - mat.Translation();
		}
		else
		{
			// 二回目以降は、前回の座標から向きを決定する
			vDir = mat.Translation() - prevPos;
		}

		// カメラからポイントへの向き
		Math::Vector3 v = mat.Translation() - mCam.Translation();
		Math::Vector3 axisX = DirectX::XMVector3Cross(vDir, v);

		float width = mat.Right().Length() * 0.5f;

		axisX.Normalize();

		// 座標
		v1.pos = mat.Translation() + axisX * width * 0.5f;
		v2.pos = mat.Translation() - axisX * width * 0.5f;

		// UV
		float uvY = i / sliceCount;

		v1.UV = { 0, uvY };
		v2.UV = { 1, uvY };

		// 座標を記憶しておく
		prevPos = mat.Translation();
	}
}

// 頂点情報をそのまま繋げてポリゴンを描画
void KdTrailPolygon::CreateVerticesWithVerticesPattern()
{
	m_vertices.clear();

	size_t pointListSize = m_pointList.size();
	if (pointListSize < 4) { return; }

	// 頂点配列
	m_vertices.resize(pointListSize);

	// 軌跡画像の分割数
	float sliceNum = pointListSize * 0.5f;

	// 頂点データ作成
	for (UINT i = 0; i < pointListSize; i++)
	{
		Vertex& v = m_vertices[i];

		// 頂点座標
		v.pos = m_pointList[i].Translation();

		// UV
		v.UV.x = (float)(i % 2);
		v.UV.y = std::clamp((i * 0.5f) / sliceNum, 0.0f, 0.99f);
	}
}
