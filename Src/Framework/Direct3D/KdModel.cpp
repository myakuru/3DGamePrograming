#include "KdModel.h"
#include "KdGLTFLoader.h"

//コンストラクター
KdModelData::KdModelData()
{
}

//デストラクター
KdModelData::~KdModelData()
{
	Release();
}

//ロード関数
bool KdModelData::Load(std::string_view filename)
{
	Release();

	std::string fileDir = KdGetDirFromPath(filename.data());
	
	std::shared_ptr<KdGLTFModel> spGltfModel = KdLoadGLTFModel(filename.data());
	if (spGltfModel == nullptr) { return false; }

	CreateNodes(spGltfModel);

	CreateMaterials(spGltfModel, fileDir);

	CreateAnimations(spGltfModel);

	return true;
}

// ノード作成
void KdModelData::CreateNodes(const std::shared_ptr<KdGLTFModel>& spGltfModel)
{
	m_originalNodes.resize(spGltfModel->Nodes.size());

	for (UINT i = 0; i < spGltfModel->Nodes.size(); i++)
	{
		// 入力元ノード
		const KdGLTFNode& rSrcNode = spGltfModel->Nodes[i];

		// 出力先のノード参照
		Node& rDstNode = m_originalNodes[i];

		if (rSrcNode.IsMesh)
		{
			// メッシュ作成
			rDstNode.m_spMesh = std::make_shared<KdMesh>();

			if (rDstNode.m_spMesh)
			{
				rDstNode.m_spMesh->Create(rSrcNode.Mesh.Vertices, rSrcNode.Mesh.Faces, rSrcNode.Mesh.Subsets, rSrcNode.Mesh.IsSkinMesh);
			}

			// メッシュノードリストにインデックス登録
			m_meshNodeIndices.push_back(i);

		}

		// ノード情報セット
		rDstNode.m_name = rSrcNode.Name;

		rDstNode.m_localTransform = rSrcNode.LocalTransform;
		rDstNode.m_worldTransform = rSrcNode.WorldTransform;
		rDstNode.m_boneInverseWorldMatrix = rSrcNode.InverseBindMatrix;

		rDstNode.m_isSkinMesh = rSrcNode.Mesh.IsSkinMesh;

		rDstNode.m_boneIndex = rSrcNode.BoneNodeIndex;

		rDstNode.m_parent = rSrcNode.Parent;
		rDstNode.m_children = rSrcNode.Children;

		// 当たり判定用ノード検索
		if (rDstNode.m_name.find("COL") != std::string::npos)
		{
			// 判定用ノードに割り当て
			m_collisionMeshNodeIndices.push_back(i);
		}
		else
		{
			// 描画ノードに割り当て
			m_drawMeshNodeIndices.push_back(i);
		}
	}

	for (UINT nodeIdx = 0; nodeIdx < spGltfModel->Nodes.size(); nodeIdx++)
	{
		// ルートノードのIndexリスト
		if (spGltfModel->Nodes[nodeIdx].Parent == -1) { m_rootNodeIndices.push_back(nodeIdx); }

		// ボーンノードのIndexリスト
		int boneIdx = spGltfModel->Nodes[nodeIdx].BoneNodeIndex;

		if (boneIdx >= 0)
		{
			if (boneIdx >= (int)m_boneNodeIndices.size()) { m_boneNodeIndices.resize(boneIdx + 1); }
			
			m_boneNodeIndices[boneIdx] = nodeIdx;
		}
	}

	// 当たり判定用ノードが1つも見つからなければ、m_drawMeshNodeと同じ割り当てを行い
	// 見た目 = 当たり判定となる
	if (!m_collisionMeshNodeIndices.size())
	{
		m_collisionMeshNodeIndices = m_drawMeshNodeIndices;
	}
}

// マテリアル作成
void KdModelData::CreateMaterials(const std::shared_ptr<KdGLTFModel>& spGltfModel, const std::string& fileDir)
{
	//マテリアル配列を受け取れるサイズのメモリを確保
	m_materials.resize(spGltfModel->Materials.size());

	for (UINT i = 0; i < m_materials.size(); ++i)
	{
		// src = sourceの略
		// dst = destinationの略
		const KdGLTFMaterial& rSrcMaterial = spGltfModel->Materials[i];
		KdMaterial& rDstMaterial = m_materials[i];

		// 名前
		rDstMaterial.m_name = rSrcMaterial.Name;

		// 各テクスチャの設定
		rDstMaterial.SetTextures(fileDir, rSrcMaterial.BaseColorTexName,
			rSrcMaterial.MetallicRoughnessTexName, rSrcMaterial.EmissiveTexName, rSrcMaterial.NormalTexName);

		// 基本色
		rDstMaterial.m_baseColorRate = rSrcMaterial.BaseColor;

		// 金属性・粗さ
		rDstMaterial.m_metallicRate = rSrcMaterial.Metallic;
		rDstMaterial.m_roughnessRate = rSrcMaterial.Roughness;

		// 自己発光・エミッシブ
		rDstMaterial.m_emissiveRate = rSrcMaterial.Emissive;
	}
}

// アニメーション作成
void KdModelData::CreateAnimations(const std::shared_ptr<KdGLTFModel>& spGltfModel)
{
	// アニメーションデータ
	m_spAnimations.resize(spGltfModel->Animations.size());

	for (UINT i = 0; i < m_spAnimations.size(); ++i)
	{
		const KdGLTFAnimationData& rSrcAnimation = *spGltfModel->Animations[i];

		m_spAnimations[i] = std::make_shared<KdAnimationData>();
		KdAnimationData& rDstAnimation = *(m_spAnimations[i]);

		rDstAnimation.m_name = rSrcAnimation.m_name;

		rDstAnimation.m_maxLength = rSrcAnimation.m_maxLength;

		rDstAnimation.m_nodes.resize(rSrcAnimation.m_nodes.size());

		for (UINT j = 0; j < rDstAnimation.m_nodes.size(); ++j)
		{
			rDstAnimation.m_nodes[j].m_nodeOffset = rSrcAnimation.m_nodes[j]->m_nodeOffset;
			rDstAnimation.m_nodes[j].m_translations = rSrcAnimation.m_nodes[j]->m_translations;
			rDstAnimation.m_nodes[j].m_rotations = rSrcAnimation.m_nodes[j]->m_rotations;
			rDstAnimation.m_nodes[j].m_scales = rSrcAnimation.m_nodes[j]->m_scales;
		}
	}
}

// アニメーションデータ取得：文字列検索
const std::shared_ptr<KdAnimationData> KdModelData::GetAnimation(std::string_view animName) const
{
	for (auto&& anim : m_spAnimations)
	{
		if (anim->m_name == animName)
		{
			return anim;
		}
	}

	return nullptr;
}

// アニメーションデータ取得：番号指定
const std::shared_ptr<KdAnimationData> KdModelData::GetAnimation(UINT index) const
{
	return index >= m_spAnimations.size() ? nullptr : m_spAnimations[index];
}

// 解放
void KdModelData::Release()
{
	m_materials.clear();
	m_originalNodes.clear();

	m_rootNodeIndices.clear();
	m_boneNodeIndices.clear();
	m_meshNodeIndices.clear();
	m_drawMeshNodeIndices.clear();
}

bool KdModelData::IsSkinMesh()
{
	for (auto& node : m_originalNodes)
	{
		if (node.m_isSkinMesh) { return true; }
	}

	return false;
}


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 
// KdModelWork
// 
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
const KdModelData::Node* KdModelWork::FindDataNode(std::string_view name) const
{
	if (m_spData == nullptr) { return nullptr; }

	return m_spData->FindNode(name.data());
}

// ノード検索：文字列
const KdModelWork::Node* KdModelWork::FindNode(std::string_view name) const
{
	for (auto&& node : m_coppiedNodes)
	{
		if (node.m_name == name.data())
		{
			return &node;
		}
	}

	return nullptr;
}

// 可変ノード検索：文字列
KdModelWork::Node* KdModelWork::FindWorkNode(std::string_view name)
{
	for (auto&& node : m_coppiedNodes)
	{
		if (node.m_name == name.data())
		{
			m_needCalcNode = true;

			return &node;
		}
	}

	return nullptr;
}

// モデル設定：コピーノードの生成
void KdModelWork::SetModelData(const std::shared_ptr<KdModelData>& rModel)
{ 
	m_spData = rModel;

	size_t nodeSize = rModel->GetOriginalNodes().size();

	m_coppiedNodes.resize(nodeSize);

	// ノードのコピーを生成
	for (UINT i = 0; i < nodeSize; ++i)
	{
		m_coppiedNodes[i].copy(rModel->GetOriginalNodes()[i]);
	}

	m_needCalcNode = true;
}

void KdModelWork::SetModelData(std::string_view fileName)
{
	// モデルのセット
	SetModelData(KdAssets::Instance().m_modeldatas.GetData(fileName));
}

// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// ルートノードから各ノードの行列を計算していく
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
void KdModelWork::CalcNodeMatrices()
{
	if (!m_spData) { assert(0 && "モデルのないノード行列計算"); return; }

	// 全ボーン行列を書き込み
	for (auto&& nodeIdx : m_spData->GetRootNodeIndices())
	{
		recCalcNodeMatrices(nodeIdx);
	}

	m_needCalcNode = false;
}

// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// ノード行列計算用の再起用関数
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
void KdModelWork::recCalcNodeMatrices(int nodeIdx, int parentNodeIdx)
{
	auto& data = m_spData->GetOriginalNodes()[nodeIdx];
	auto& work = m_coppiedNodes[nodeIdx];

	// 親との行列を合成
	if (parentNodeIdx >= 0)
	{
		auto& parent = m_coppiedNodes[data.m_parent];

		work.m_worldTransform = work.m_localTransform * parent.m_worldTransform;
	}
	// 親が居ない場合は親は自分自身とする
	else
	{
		work.m_worldTransform = work.m_localTransform;
	}

	for (auto childNodeIdx : data.m_children)
	{
		recCalcNodeMatrices(childNodeIdx, nodeIdx);
	}
}
