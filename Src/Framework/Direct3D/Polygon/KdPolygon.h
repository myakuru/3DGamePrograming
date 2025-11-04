#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ポリゴンの共通情報を扱う基底クラス
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// それぞれの派生クラスにおいて描画の際にポリゴンを形成する頂点情報を作成する機能を持たせる必要がある
// ポリゴンに反映させるマテリアルの情報を保持している
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdPolygon
{
public:
	struct Vertex
	{
		Math::Vector3	pos;
		Math::Vector2	UV;
		unsigned int	color		= 0xFFFFFFFF;
		Math::Vector3	normal		= Math::Vector3::Backward;
		Math::Vector3	tangent		= Math::Vector3::Left;

		float			_blank[6]	= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	};

	KdPolygon() {}
	KdPolygon(const std::shared_ptr<KdTexture>& spBaseColTex) { SetMaterial(spBaseColTex); }
	KdPolygon(const std::string& baseColTexName) { SetMaterial(baseColTexName); }
	virtual ~KdPolygon(){}

	// 描画時に使用する頂点リストの取得
	const std::vector<Vertex>& GetVertices() const { return m_vertices; };
	// 判定用ポリゴン頂点座標のコピー
	void GetPositions(std::vector<Math::Vector3>& res) const;

	// マテリアルの設定
	void SetMaterial(const std::shared_ptr<KdMaterial>& spMaterial) { m_spMaterial = spMaterial; }
	// ベースカラーテクスチャから
	void SetMaterial(const std::shared_ptr<KdTexture>& spBaseColTex);
	// ベースカラーファイルパスから
	void SetMaterial(const std::string& baseColTexName);

	virtual void SetColor(const Math::Color& col) { if (m_spMaterial) { m_spMaterial->m_baseColorRate = col; } }

	// マテリアルを取得
	const std::shared_ptr<KdMaterial>& GetMaterial() const { return m_spMaterial; }

	bool IsEnable() const { return (m_enable); }
	void SetEnable(bool enable) { m_enable = enable; }

	bool Is2DObject() const { return m_2DObject; }
	void Set2DObject(bool is2DObject) { m_2DObject = is2DObject; }

protected:

	// ポリゴンに描画するテクスチャ
	std::shared_ptr<KdMaterial> m_spMaterial = nullptr;

	// 頂点リスト
	std::vector<Vertex>		m_vertices;

	// 描画有効フラグ
	bool m_enable = true;

	// 2Dオブジェクトかどうか
	bool m_2DObject = true;
};
