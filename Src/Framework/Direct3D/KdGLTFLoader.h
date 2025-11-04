#pragma once

//=====================================================
//
// GLTF形式の3Dモデルファイルを読み込む
//
//=====================================================

//============================
// マテリアル
//============================
struct KdGLTFMaterial
{
	std::string		Name;						// マテリアル名


	std::string		AlphaMode = "OPAQUE";		// "OPAQUE" : レンダリングされた出力は完全に不透明で、アルファ値は無視されます。
												// "MASK"   : レンダリングされた出力は、アルファ値と指定されたアルファカットオフ値に応じて、完全に不透明または完全に透明になります。このモードは、木の葉やワイヤーフェンスなどのジオメトリをシミュレートするために使用されます。
												// "BLEND"  : 

	float			AlphaCutoff = 0.5f;			// MASKモード時に、カットオフの閾値として使用　それ以外のモードでは使用されない
	bool			DoubleSided = true;		// 両面か？

	//------------------------------
	// PBR材質データ
	//------------------------------
	std::string		BaseColorTexName;			// 基本色テクスチャのファイル名
	Math::Vector4	BaseColor = { 1,1,1,1 };	// 上記テクスチャのRGBAのスケーリング要素

	// 金属性、粗さ
	std::string		MetallicRoughnessTexName;	// メタリックとラフネスのテクスチャ　青成分 = メタリック 緑成分 = ラフネス
	float			Metallic = 1.0f;			// 上記テクスチャのメタリック要素の乗算用　テクスチャが無い時は乗算ではなくそのまま使用する
	float			Roughness = 1.0f;			// 上記テクスチャのラフネス要素の乗算用　テクスチャが無い時は乗算ではなくそのまま使用する

	// エミッシブ：自己発光 つまり表面から放出される光　RGBのみ使用
	std::string		EmissiveTexName;			// エミッシブテクスチャ　RGBを使用
	Math::Vector3	Emissive = { 0,0,0 };		// 上記テクスチャのRGBのスケーリング要素

	//------------------------------
	// その他テクスチャ
	//------------------------------
	std::string		NormalTexName;				// 法線マップテクスチャ
	std::string		OcclusionTexName;			// 光の遮蔽度テクスチャ　赤成分のみ使用
};

//============================
// ノード １つのメッシュやマテリアルなど
//============================
struct KdGLTFNode
{
	//---------------------------
	// 基本情報
	//---------------------------

	// 名前
	std::string								Name;

	// 子Indexリスト
	std::vector<int>						Children;
	// 親Index
	int										Parent = -1;
	// ボーンの場合のIndex
	int										BoneNodeIndex = -1;

	// 行列
	Math::Matrix							LocalTransform;
	Math::Matrix							WorldTransform;
	Math::Matrix							InverseBindMatrix;	// ボーンオフセット行列

	//---------------------------
	// Mesh専用情報
	//---------------------------
	bool									IsMesh = false;
	struct Mesh
	{
		// 頂点配列
		std::vector<KdMeshVertex>				Vertices;
		// 面情報配列
		std::vector<KdMeshFace>					Faces;
		// サブセット情報配列
		std::vector<KdMeshSubset>				Subsets;

		bool									IsSkinMesh = false;
	};
	Mesh									Mesh;

};

//============================
// アニメーションデータ
//============================
struct KdGLTFAnimationData
{
	// アニメーション名
	std::string				m_name;
	// アニメの長さ
	float					m_maxLength = 0;
	// １ノードのアニメーションデータ
	struct Node
	{
		int								m_nodeOffset = -1;	// 対象ノードのOffset
		// 各チャンネル
		std::vector<KdAnimKeyVector3>		m_translations;	// 位置キーリスト
		std::vector<KdAnimKeyQuaternion>	m_rotations;	// 回転キーリスト
		std::vector<KdAnimKeyVector3>		m_scales;		// 拡大キーリスト
	};
	// 全ノード用アニメーションデータ
	std::vector<std::shared_ptr<Node>>	m_nodes;
};

//============================
// モデルデータ
//============================
struct KdGLTFModel
{
	// 全ノードデータ
	std::vector<KdGLTFNode>						Nodes;

	// 全ノード中のルートノードのみのIndexリスト
	std::vector<int>							RootNodeIndices;

	// 全ノード中のボーンノードだけのIndexリスト
	std::vector<int>							BoneNodeIndices;

	// マテリアル一覧
	std::vector<KdGLTFMaterial>					Materials;

	// アニメーションデータリスト
	std::vector<std::shared_ptr<KdGLTFAnimationData>>	Animations;
};


//===================================================
// GLTF形式の3Dモデルを読み込む
// LoaderはTinygltfを使用しています。
// github:https://github.com/syoyo/tinygltf
// 
// ・path				… .glflファイルのパス
//===================================================
std::shared_ptr<KdGLTFModel> KdLoadGLTFModel(std::string_view path);
