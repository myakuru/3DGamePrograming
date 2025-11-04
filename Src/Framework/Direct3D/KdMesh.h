#pragma once

//==========================================================
// メッシュ用 頂点情報
//==========================================================
struct KdMeshVertex
{
	Math::Vector3	Pos;		// 座標
	Math::Vector2	UV;			// UV
	unsigned int	Color = 0xFFFFFFFF;			// RGBA色(容量削減のため、各色0～255のUINT型)
	Math::Vector3	Normal;		// 法線
	Math::Vector3	Tangent;	// 接線

	std::array<short, 4>	SkinIndexList;		// スキニングIndexリスト
	std::array<float, 4>	SkinWeightList;		// スキニングウェイトリスト
};

//==========================================================
// メッシュ用 面情報
//==========================================================
struct KdMeshFace
{
	UINT Idx[3];				// 三角形を構成する頂点のIndex
};

//==========================================================
// メッシュ用 サブセット情報
//==========================================================
struct KdMeshSubset
{
	UINT		MaterialNo = 0;		// マテリアルNo

	UINT		FaceStart = 0;		// 面Index　このマテリアルで使用されている最初の面のIndex
	UINT		FaceCount = 0;		// 面数　FaceStartから、何枚の面が使用されているかの
};

//==========================================================
//
// メッシュクラス
//
//==========================================================
class KdMesh
{
public:

	//=================================================
	// 取得・設定
	//=================================================

	// サブセット情報配列を取得
	const std::vector<KdMeshSubset>&	GetSubsets() const { return m_subsets; }

	// 頂点の座標配列を取得
	const std::vector<Math::Vector3>&	GetVertexPositions() const { return m_positions; }
	// 面の配列を取得
	const std::vector<KdMeshFace>&		GetFaces() const { return m_faces; }

	// 軸平行境界ボックス取得
	const DirectX::BoundingBox&			GetBoundingBox() const { return m_aabb; }
	// 境界球取得
	const DirectX::BoundingSphere&		GetBoundingSphere() const { return m_bs; }

	// メッシュデータをデバイスへセットする
	void SetToDevice() const;

	// スキンメッシュ？
	bool IsSkinMesh() const { return m_isSkinMesh; }

	//=================================================
	// 作成・解放
	//=================================================

	// メッシュ作成
	// ・vertices		… 頂点配列
	// ・faces			… 面インデックス情報配列
	// ・subsets		… サブセット情報配列
	// 戻り値			… 成功：true
	bool Create(const std::vector<KdMeshVertex>& vertices, const std::vector<KdMeshFace>& faces, const std::vector<KdMeshSubset>& subsets, bool isSkinMesh);

	// 解放
	void Release()
	{
		m_vertBuf.Release();
		m_indxBuf.Release();
		m_subsets.clear();
		m_positions.clear();
		m_faces.clear();
	}

	~KdMesh()
	{
		Release();
	}

	//=================================================
	// 処理
	//=================================================

	// 指定サブセットを描画
	void DrawSubset(int subsetNo) const;

	// 
	KdMesh() {}

private:

	// 頂点バッファ
	KdBuffer					m_vertBuf;
	// インデックスバッファ
	KdBuffer					m_indxBuf;

	// サブセット情報
	std::vector<KdMeshSubset>	m_subsets;

	// 境界データ
	DirectX::BoundingBox		m_aabb;	// 軸平行境界ボックス
	DirectX::BoundingSphere		m_bs;	// 境界球

	// 座標のみの配列(複製)
	std::vector<Math::Vector3>	m_positions;
	// 面情報のみの配列(複製)
	std::vector<KdMeshFace>		m_faces;

	bool						m_isSkinMesh = false;

private:
	// コピー禁止用
	KdMesh(const KdMesh& src) = delete;
	void operator=(const KdMesh& src) = delete;
};
