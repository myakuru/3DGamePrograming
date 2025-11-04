#include "Framework/KdFramework.h"

#include "KdGLTFLoader.h"

// TinyGLTF
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "tiny_gltf.h"

// GLTFのデバッグ表示を有効
//#define GLTF_DEBUG

static void Dump(const tinygltf::Model &model);

//===================================================
// ファイル名から拡張子を取得
//===================================================
static std::string GetFilePathExtension(const std::string &FileName)
{
	if (FileName.find_last_of(".") != std::string::npos)
	{
		return FileName.substr(FileName.find_last_of(".") + 1);
	}
	return "";
}


//===================================================
// バッファから型を指定して取得する関数
//===================================================
class GLTFBufferGetter {
public:

	//	GLTFBufferGetter(const BYTE* address) : m_address(address) { }

	GLTFBufferGetter(const tinygltf::Model* model, int accessor)
	{
		m_model = model;

		m_accessor = &model->accessors[accessor];
		// バッファビュー
		m_bufferView = &model->bufferViews[m_accessor->bufferView];
		// バッファ
		m_buffer = &model->buffers[m_bufferView->buffer];

		m_address = &m_buffer->data[m_bufferView->byteOffset + m_accessor->byteOffset];
	}


	// Float取得
	float GetValue_Float(int index)
	{
		if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_BYTE)					return Get<char>(index) / (float)SCHAR_MAX;
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE)	return Get<BYTE>(index) / (float)UCHAR_MAX;
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_SHORT)			return Get<short>(index) / (float)SHRT_MAX;
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)	return Get<unsigned short>(index) / (float)USHRT_MAX;
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_INT)				return Get<int>(index) / (float)INT_MAX;
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)		return Get<unsigned int>(index) / (float)UINT_MAX;
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_FLOAT)			return Get<float>(index);

		assert(0 && "対応していない型");
		return 0;
	}

	// 整数取得
	int GetValue_Int(int index)
	{
		if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_BYTE)					return (int)Get<char>(index);
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE)	return (int)Get<BYTE>(index);
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_SHORT)			return (int)Get<short>(index);
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)	return (int)Get<unsigned short>(index);
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_INT)				return (int)Get<int>(index);
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)		return (int)Get<unsigned int>(index);

		assert(0 && "対応していない型");
		return 0;
	}

	// 値を正規化して取得
	float GetValue_UNORM(int index)
	{
		if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_BYTE)
		{
			return std::max(Get<char>(index) / 127.0f, -1.0f);
		}
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE)
		{
			return Get<BYTE>(index) / 255.0f;
		}
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_SHORT)
		{
			return std::max(Get<short>(index) / 32767.0f, -1.0f);
		}
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
		{
			return Get<unsigned short>(index) / 65535.0f;
		}
		else if (m_accessor->componentType == TINYGLTF_PARAMETER_TYPE_FLOAT)
		{
			return GetValue_Float(index);
		}

		assert(0 && "対応していない型");
		return 0;
	}

	// 
	const tinygltf::Accessor*	GetAccessor() const { return m_accessor; }
	const tinygltf::BufferView*	GetBufferView() const { return m_bufferView; }
	const tinygltf::Buffer*		GetBuffer() const { return m_buffer; }


private:

	// 指定型でindex番目のデータを取得
	template<class Type>
	const Type& Get(int index) {
		return *(const Type*)&m_address[index * sizeof(Type)];
	}

	const BYTE* m_address = nullptr;

	const tinygltf::Model*		m_model = nullptr;
	const tinygltf::Accessor*	m_accessor = nullptr;
	const tinygltf::BufferView*	m_bufferView = nullptr;
	const tinygltf::Buffer*		m_buffer = nullptr;
};

//===================================================
// 行列のZ軸ミラーリング
//===================================================
// 参考：http://momose-d.cocolog-nifty.com/blog/2014/08/post-0735.html
static void MatrixMirrorZ(Math::Matrix& mat)
{
	// 回転のZミラーリング
	mat._13 *= -1;
	mat._23 *= -1;
	mat._31 *= -1;
	mat._32 *= -1;
	// 座標のZミラーリング
	mat._43 *= -1;
}

//===================================================
// GLTF形式の3Dモデルを読み込む
// ※左手座標系にするため下記の仕様でZ軸反転も行う(アニメーションやボーンを使用するときも同様にすること)
// 　・行列：MatrixMirrorZ関数で反転
// 　・クォータニオン：xとyに-1を乗算
// 　・座標：zに-1を乗算
//===================================================
std::shared_ptr<KdGLTFModel> KdLoadGLTFModel(std::string_view path)
{
#ifdef GLTF_DEBUG
	// コンソールウィンドウ表示
	if (AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
	}
#endif

	tinygltf::Model model;
	{
		tinygltf::TinyGLTF gltf_ctx;
		std::string err;
		std::string warn;
		std::string input_filename(path);
		std::string ext = GetFilePathExtension(input_filename);

		// GLTF読み込み
		bool ret = false;
		if (ext.compare("glb") == 0) {
			std::cout << "Reading binary glTF" << std::endl;
			// assume binary glTF.
			ret = gltf_ctx.LoadBinaryFromFile(&model, &err, &warn, input_filename.c_str());
		}
		else {
			std::cout << "Reading ASCII glTF" << std::endl;
			// assume ascii glTF.
			ret = gltf_ctx.LoadASCIIFromFile(&model, &err, &warn, input_filename.c_str());
		}

		if (!warn.empty()) {
			printf("Warn: %s\n", warn.c_str());
		}

		if (!err.empty()) {
			printf("Err: %s\n", err.c_str());
		}

		if (!ret) {
			printf("Failed to parse glTF\n");
			return nullptr;
		}
	}

#ifdef GLTF_DEBUG
	// 情報表示
	Dump(model);
#endif

	std::shared_ptr<KdGLTFModel>	destModel = std::make_shared<KdGLTFModel>();

	//----------------------------------
	// マテリアル
	//----------------------------------
	{
		// 指定Indexのテクスチャ名取得
		auto GetTextureFilename = [&model](int texIndex) -> std::string
		{
			if (texIndex < 0)return "";
			int imgIndex = model.textures[texIndex].source;
			if (imgIndex < 0)return "";
			return model.images[imgIndex].uri;
		};

		// マテリアル数だけ、配列確保
		destModel->Materials.resize(model.materials.size());
		// 全マテリアルデータをコピー
		for (UINT matei = 0; matei < destModel->Materials.size(); matei++)
		{
			const auto& srcMaterial = model.materials[matei];
			auto& destMaterial = destModel->Materials[matei];

			// 名前
			destMaterial.Name = srcMaterial.name;

			// フラグ系
			destMaterial.AlphaMode = srcMaterial.alphaMode;
			destMaterial.AlphaCutoff = (float)srcMaterial.alphaCutoff;
			destMaterial.DoubleSided = srcMaterial.doubleSided;

			// 基本色
			destMaterial.BaseColorTexName = GetTextureFilename(srcMaterial.pbrMetallicRoughness.baseColorTexture.index);
			if (srcMaterial.pbrMetallicRoughness.baseColorFactor.size() == 4)
			{
				destMaterial.BaseColor.x = (float)srcMaterial.pbrMetallicRoughness.baseColorFactor[0];
				destMaterial.BaseColor.y = (float)srcMaterial.pbrMetallicRoughness.baseColorFactor[1];
				destMaterial.BaseColor.z = (float)srcMaterial.pbrMetallicRoughness.baseColorFactor[2];
				destMaterial.BaseColor.w = (float)srcMaterial.pbrMetallicRoughness.baseColorFactor[3];
			}

			// 金属性、粗さ
			destMaterial.MetallicRoughnessTexName = GetTextureFilename(srcMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index);
			destMaterial.Metallic = (float)srcMaterial.pbrMetallicRoughness.metallicFactor;
			destMaterial.Roughness = (float)srcMaterial.pbrMetallicRoughness.roughnessFactor;

			// エミッシブ
			destMaterial.EmissiveTexName = GetTextureFilename(srcMaterial.emissiveTexture.index);
			if (srcMaterial.emissiveFactor.size() == 3)
			{
				destMaterial.Emissive.x = (float)srcMaterial.emissiveFactor[0];
				destMaterial.Emissive.y = (float)srcMaterial.emissiveFactor[1];
				destMaterial.Emissive.z = (float)srcMaterial.emissiveFactor[2];
			}

			// 法線マップ
			destMaterial.NormalTexName = GetTextureFilename(srcMaterial.normalTexture.index);
			// オクルージョンマップ
			destMaterial.OcclusionTexName = GetTextureFilename(srcMaterial.occlusionTexture.index);
		}

		// マテリアルがゼロの場合は、１つだけ作成しておく
		if (destModel->Materials.size() == 0)destModel->Materials.resize(1);

	}

	//----------------------------------
	// 全ノードを取得し、基本的なデータを作成する
	//----------------------------------

	// 全ノードぶんメモリ確保
	destModel->Nodes.resize(model.nodes.size());

	//----------------------------------
	// 全ノード 基本情報設定
	//----------------------------------
	for (UINT nodei = 0; nodei < destModel->Nodes.size(); nodei++)
	{
		auto* destNode = &destModel->Nodes[nodei];

		//----------------------------
		// 情報
		//----------------------------

		// 名前
		destNode->Name = model.nodes[nodei].name;

		// 子インデックス配列
		destNode->Children = model.nodes[nodei].children;

		// 全ての子に、親設定
		for (auto&& idx : destNode->Children)
		{
			destModel->Nodes[idx].Parent = nodei;
		}

		//----------------------------
		// 変換行列取得
		//----------------------------
		Math::Matrix mS, mR, mT;
		// 拡大
		if (model.nodes[nodei].scale.size() != 0)
		{
			mS = Math::Matrix::CreateScale(
				(float)model.nodes[nodei].scale[0],
				(float)model.nodes[nodei].scale[1],
				(float)model.nodes[nodei].scale[2]
			);
		}
		// 回転
		if (model.nodes[nodei].rotation.size() != 0)
		{
			Math::Quaternion q(
				(float)model.nodes[nodei].rotation[0],
				(float)model.nodes[nodei].rotation[1],
				(float)model.nodes[nodei].rotation[2],
				(float)model.nodes[nodei].rotation[3]
			);
			mR = Math::Matrix::CreateFromQuaternion(q);
		}
		// 移動
		if (model.nodes[nodei].translation.size() != 0)
		{
			mT = Math::Matrix::CreateTranslation(
				(float)model.nodes[nodei].translation[0],
				(float)model.nodes[nodei].translation[1],
				(float)model.nodes[nodei].translation[2]
			);
		}
		// 行列
		if (model.nodes[nodei].matrix.size() != 0)
		{
			for (int n = 0; n < 16; n++)
			{
				*(&mS._11 + n) = (float)model.nodes[nodei].matrix[n];
			}
		}

		// 変換行列
		destNode->LocalTransform = mS * mR * mT;
		// Z軸ミラー
		MatrixMirrorZ(destNode->LocalTransform);

		// メッシュあり
		if (model.nodes[nodei].mesh >= 0)
		{
			// MeshフラグOn
			destNode->IsMesh = true;
		}
	}

	//----------------------------------
	// ルートノードのみの参照リスト
	//----------------------------------
	for (auto&& idx : model.scenes[0].nodes)
	{
		destModel->RootNodeIndices.push_back(idx);
	}

	//----------------------------------
	// 各ノードのTransformからWorldTransformを算出
	//----------------------------------
	{
		// 行列計算用 再帰関数
		std::function<void(KdGLTFNode*, const Math::Matrix*)> rec = [&rec, &destModel](KdGLTFNode* node, const Math::Matrix* parentMat)
		{
			if (parentMat) {
				node->WorldTransform = node->LocalTransform * (*parentMat);
			}
			else {
				node->WorldTransform = node->LocalTransform;
			}

			// 子再帰
			for (auto&& child : node->Children)
			{
				rec(&destModel->Nodes[child], &node->WorldTransform);
			}
		};

		// 親子関係から行列を作成
		for (int nodeIdx : destModel->RootNodeIndices)
		{
			rec(&destModel->Nodes[nodeIdx], nullptr);
		}
	}

	//----------------------------------
	// ボーン
	//----------------------------------
	if (model.skins.size() > 0)
	{
		// 配列確保
		destModel->BoneNodeIndices = model.skins[0].joints;

		// inverseBindMarices(オフセット行列)取得用
		GLTFBufferGetter ibmGetter(&model, model.skins[0].inverseBindMatrices);

		// ボーンだけのノード参照配列
		// ※頂点のSkinIndexは、このIndexになるようです
		for (UINT ji = 0; ji < model.skins[0].joints.size(); ji++)
		{
			// ji番目のボーンの、ノード内でのIndex
			int nodeIdx = model.skins[0].joints[ji];

			KdGLTFNode* boneNode = &destModel->Nodes[nodeIdx];
			boneNode->BoneNodeIndex = ji;

			// オフセット行列取得
			Math::Matrix invBindMat;
			for (int mati = 0; mati < 16; mati++)
			{
				(&invBindMat._11)[mati] = ibmGetter.GetValue_Float(ji * 16 + mati);
			}
			MatrixMirrorZ(invBindMat);
			boneNode->InverseBindMatrix = invBindMat;
			// 変換行列へ変換
			boneNode->WorldTransform = invBindMat.Invert();
		}

		// ボーンLocalMat算出
		for (int nodeIdx : destModel->BoneNodeIndices)
		{
			KdGLTFNode* boneNode = &destModel->Nodes[nodeIdx];

			if (boneNode->Parent >= 0)
			{
				boneNode->LocalTransform = boneNode->WorldTransform * destModel->Nodes[boneNode->Parent].InverseBindMatrix;
			}
			else
			{
				boneNode->LocalTransform = boneNode->WorldTransform;
			}
		}
	}

	//----------------------------------
	// メッシュ
	//----------------------------------
	for (UINT nodei = 0; nodei < destModel->Nodes.size(); nodei++)
	{
		auto* destNode = &destModel->Nodes[nodei];

		//--------------------------------
		// メッシュの場合
		//--------------------------------
		// メッシュIndex
		int msi = model.nodes[nodei].mesh;
		if (msi < 0)continue;	// メッシュなし

		// MeshフラグOn
		destNode->IsMesh = true;

		// 作業データ
		struct GLTFPrimitive
		{
			std::vector<KdMeshVertex>			Vertices;
			std::vector<KdMeshFace>				Faces;

			UINT								MaterialNo = 0;

			std::map<std::string, int>			Attributes;
		};
		std::vector<std::shared_ptr<GLTFPrimitive>>	tempPrimitives(model.meshes[msi].primitives.size());

		// 全プリミティブ(Subset)
		for (size_t pri = 0; pri < model.meshes[msi].primitives.size(); pri++)
		{
			auto& srcPrimitive = model.meshes[msi].primitives[pri];

			// 今回はTRIANGLES以外は無視する
			if (srcPrimitive.mode != TINYGLTF_MODE_TRIANGLES)continue;

			// 作成
			std::shared_ptr<GLTFPrimitive> destPrimitive = std::make_shared<GLTFPrimitive>();
			tempPrimitives[pri] = destPrimitive;
			destPrimitive->Attributes = srcPrimitive.attributes;

			// マテリアルNo
			destPrimitive->MaterialNo = std::max(0, srcPrimitive.material);

			// 頂点バッファ
			{
				// 座標
				{
					// 座標ゲッター
					GLTFBufferGetter posGetter(&model, srcPrimitive.attributes["POSITION"]);

					Math::Vector3 pos;
					destPrimitive->Vertices.resize(posGetter.GetAccessor()->count);
					for (UINT vi = 0; vi < posGetter.GetAccessor()->count; vi++) {
						auto& ver = destPrimitive->Vertices[vi];

						if (posGetter.GetAccessor()->type != TINYGLTF_TYPE_VEC3) {
							assert(0 && "この頂点形式には対応してません");
						}

						ver.Pos.x = posGetter.GetValue_Float(vi * 3 + 0);
						ver.Pos.y = posGetter.GetValue_Float(vi * 3 + 1);
						ver.Pos.z = posGetter.GetValue_Float(vi * 3 + 2) * -1;
					}
				}

				// 法線
				if (srcPrimitive.attributes.count("NORMAL") > 0)
				{
					// 法線ゲッター
					GLTFBufferGetter normalGetter(&model, srcPrimitive.attributes["NORMAL"]);

					for (UINT vi = 0; vi < destPrimitive->Vertices.size(); vi++) {
						auto& nor = destPrimitive->Vertices[vi].Normal;
						nor.x = normalGetter.GetValue_Float(vi * 3 + 0);
						nor.y = normalGetter.GetValue_Float(vi * 3 + 1);
						nor.z = normalGetter.GetValue_Float(vi * 3 + 2) * -1;
					}
				}

				// UV
				if (srcPrimitive.attributes.count("TEXCOORD_0") > 0)
				{
					// UVゲッター
					GLTFBufferGetter uvGetter(&model, srcPrimitive.attributes["TEXCOORD_0"]);

					for (UINT vi = 0; vi < destPrimitive->Vertices.size(); vi++) {
						auto& uv = destPrimitive->Vertices[vi].UV;

						uv.x = uvGetter.GetValue_UNORM(vi * 2 + 0);
						uv.y = uvGetter.GetValue_UNORM(vi * 2 + 1);
					}
				}

				// 頂点カラー
				if (srcPrimitive.attributes.count("COLOR_0") > 0)
				{
					// 色ゲッター
					GLTFBufferGetter colorGetter(&model, srcPrimitive.attributes["COLOR_0"]);

					for (UINT vi = 0; vi < destPrimitive->Vertices.size(); vi++)
					{
						Math::Color color(1,1,1,1);

						// RGB
						if (colorGetter.GetAccessor()->type == TINYGLTF_TYPE_VEC3)
						{
							color.x = colorGetter.GetValue_Float(vi * 3 + 0);
							color.y = colorGetter.GetValue_Float(vi * 3 + 1);
							color.z = colorGetter.GetValue_Float(vi * 3 + 2);
						}
						// RGBA
						else if (colorGetter.GetAccessor()->type == TINYGLTF_TYPE_VEC4)
						{
							color.x = colorGetter.GetValue_Float(vi * 4 + 0);
							color.y = colorGetter.GetValue_Float(vi * 4 + 1);
							color.z = colorGetter.GetValue_Float(vi * 4 + 2);
							color.w = colorGetter.GetValue_Float(vi * 4 + 3);
						}

						destPrimitive->Vertices[vi].Color = color.RGBA().v;
					}
				}

				// スキンメッシュ情報が無ければ現状不要なので無視
				if (model.skins.size() > 0)
				{
					// Skin INDEX
					if (srcPrimitive.attributes.count("JOINTS_0") > 0)
					{
						destNode->Mesh.IsSkinMesh = true;

						GLTFBufferGetter jointGetter(&model, srcPrimitive.attributes["JOINTS_0"]);

						for (UINT vi = 0; vi < destPrimitive->Vertices.size(); vi++)
						{
							// ※IndexはボーンリストのIndexになる(ノード全体ではない)
							auto& skinIndex = destPrimitive->Vertices[vi].SkinIndexList;

							skinIndex[0] = (short)jointGetter.GetValue_Int(vi * 4 + 0);
							skinIndex[1] = (short)jointGetter.GetValue_Int(vi * 4 + 1);
							skinIndex[2] = (short)jointGetter.GetValue_Int(vi * 4 + 2);
							skinIndex[3] = (short)jointGetter.GetValue_Int(vi * 4 + 3);
						}
					}

					// Skin WEIGHT
					if (srcPrimitive.attributes.count("WEIGHTS_0") > 0)
					{
						destNode->Mesh.IsSkinMesh = true;

						GLTFBufferGetter weightGetter(&model, srcPrimitive.attributes["WEIGHTS_0"]);

						for (UINT vi = 0; vi < destPrimitive->Vertices.size(); vi++)
						{
							auto& skinWei = destPrimitive->Vertices[vi].SkinWeightList;

							skinWei[0] = weightGetter.GetValue_UNORM(vi * 4 + 0);
							skinWei[1] = weightGetter.GetValue_UNORM(vi * 4 + 1);
							skinWei[2] = weightGetter.GetValue_UNORM(vi * 4 + 2);
							skinWei[3] = weightGetter.GetValue_UNORM(vi * 4 + 3);

							if (skinWei[0] == 0)skinWei[0] = 1.0f;

							// ウェイト正規化
							int cnt = 0;
							for (UINT x = 0; x < 4; x++)
							{
								if (skinWei[x] == 0.0f)break;
								cnt++;
							}
							float totalW = 0;
							for (int x = 0; x < cnt - 1; x++)
							{
								totalW += skinWei[x];
							}
							skinWei[cnt - 1] = 1.0f - totalW;
						}
					}
				}
			}

			// インデックスバッファ
			{
				GLTFBufferGetter indexGetter(&model, srcPrimitive.indices);

				// 面数ぶんリサイズ
				destPrimitive->Faces.resize(indexGetter.GetAccessor()->count / 3);
				for (UINT di = 0; di < destPrimitive->Faces.size(); di++)
				{
					// データ型のバイト数求める(Z軸ミラーのため、1と2を入れ替えています)
					destPrimitive->Faces[di].Idx[0] = (UINT)indexGetter.GetValue_Int(di * 3 + 0);
					destPrimitive->Faces[di].Idx[2] = (UINT)indexGetter.GetValue_Int(di * 3 + 1);
					destPrimitive->Faces[di].Idx[1] = (UINT)indexGetter.GetValue_Int(di * 3 + 2);
				}
			}
		}

		// マテリアルソート
		std::sort(
			tempPrimitives.begin(),
			tempPrimitives.end(),
			[](std::shared_ptr<GLTFPrimitive> v1, std::shared_ptr<GLTFPrimitive> v2) {
				return v1->MaterialNo < v2->MaterialNo;
			}
		);

		// マテリアルの最大数ぶんサブセット作成
		destNode->Mesh.Subsets.resize(tempPrimitives.size());
		for (UINT pi = 0; pi < tempPrimitives.size(); pi++)
		{
			// マテリアル番号
			destNode->Mesh.Subsets[pi].MaterialNo = tempPrimitives[pi]->MaterialNo;
		}

		// 全プリミティブを合成し、１つのメッシュにする
		UINT currentVertexIdx	= 0;
		UINT currentFaceIdx		= 0;
		for (UINT pi = 0; pi < tempPrimitives.size(); pi++)
		{
			const auto& prim = tempPrimitives[pi];

			// 頂点バッファ合成
			if (prim->Vertices.size() >= 1) {
				UINT st = (UINT)destNode->Mesh.Vertices.size();
				destNode->Mesh.Vertices.resize(destNode->Mesh.Vertices.size() + prim->Vertices.size());
				memcpy(&destNode->Mesh.Vertices[st], &prim->Vertices[0], prim->Vertices.size() * sizeof(KdMeshVertex));
			}

			// インデックス合成
			if (prim->Faces.size() >= 1) {
				UINT st = (UINT)destNode->Mesh.Faces.size();
				destNode->Mesh.Faces.resize(destNode->Mesh.Faces.size() + prim->Faces.size());
				// 反転するため 0, 2, 1の順番にする(通常は0, 1, 2の順番)
				for (UINT fi = 0; fi < prim->Faces.size(); fi++) {
					destNode->Mesh.Faces[st + fi].Idx[0] = prim->Faces[fi].Idx[0] + currentVertexIdx;
					destNode->Mesh.Faces[st + fi].Idx[1] = prim->Faces[fi].Idx[1] + currentVertexIdx;
					destNode->Mesh.Faces[st + fi].Idx[2] = prim->Faces[fi].Idx[2] + currentVertexIdx;
				}
			}

			// Subset
			destNode->Mesh.Subsets[pi].FaceCount += (UINT)prim->Faces.size();	// 面数を加算

			// 
			currentVertexIdx	+= (UINT)prim->Vertices.size();
			currentFaceIdx		+= (UINT)prim->Faces.size();

		}
		tempPrimitives.clear();

		// サブセットのオフセットを求める
		{
			UINT offset = 0;
			for (UINT pi = 0; pi < destNode->Mesh.Subsets.size(); pi++)
			{
				destNode->Mesh.Subsets[pi].FaceStart = offset;	// 開始Index

				offset += destNode->Mesh.Subsets[pi].FaceCount;
			}
		}

		// メッシュの全頂点の接線を計算する
		for (auto&& v : destNode->Mesh.Vertices)
		{
			// 接線が存在する場合はスキップ
			if (v.Tangent.Length()) { continue; }

			Math::Vector3( 0.0f, 1.0f, 0.0f ).Cross(v.Normal, v.Tangent);
			
			if (v.Tangent.x == 0 && v.Tangent.y == 0 && v.Tangent.z == 0)
			{
				Math::Vector3( 0.0f, 0.0f, -1.0f).Cross(v.Normal, v.Tangent);
			}
		}
	}

	//----------------------------------
	// アニメーション
	//----------------------------------
	for (UINT ani = 0; ani < model.animations.size(); ani++)
	{
		const auto& srcAni = model.animations[ani];

		std::shared_ptr<KdGLTFAnimationData>	animation = std::make_shared<KdGLTFAnimationData>();
		destModel->Animations.push_back(animation);

		// 名前
		animation->m_name = srcAni.name;

		// 
		std::vector<std::shared_ptr<KdGLTFAnimationData::Node>> tempNodes;
		tempNodes.resize(destModel->Nodes.size());

		// 全チャンネル
		for (const auto& channel : srcAni.channels)
		{
			const auto& sampler = srcAni.samplers[channel.sampler];

			// 対象ノードのIndex
			auto& destAnimNode = tempNodes[channel.target_node];

			// 初回
			if (destAnimNode == nullptr)
			{
				destAnimNode = std::make_shared<KdGLTFAnimationData::Node>();
				destAnimNode->m_nodeOffset = channel.target_node;
			}

			// 時間アクセサ
			GLTFBufferGetter timeGetter(&model, sampler.input);
			// データアクセサ
			GLTFBufferGetter valueGetter(&model, sampler.output);

			if (channel.target_path == "translation")
			{

				for (UINT ki = 0; ki < timeGetter.GetAccessor()->count; ki++)
				{
					KdAnimKeyVector3 v;
					// 時間
					v.m_time = timeGetter.GetValue_Float(ki) * 60.0f;	// 元が60fpsとして変換
					if (v.m_time > animation->m_maxLength)
					{
						animation->m_maxLength = v.m_time;
					}

					// 値
					if (sampler.interpolation == "STEP")
					{
						v.m_vec.x = valueGetter.GetValue_Float(ki * 3 + 0);
						v.m_vec.y = valueGetter.GetValue_Float(ki * 3 + 1);
						v.m_vec.z = valueGetter.GetValue_Float(ki * 3 + 2) * -1;
						destAnimNode->m_translations.push_back(v);
					}
					else if (sampler.interpolation == "LINEAR")
					{
						v.m_vec.x = valueGetter.GetValue_Float(ki * 3 + 0);
						v.m_vec.y = valueGetter.GetValue_Float(ki * 3 + 1);
						v.m_vec.z = valueGetter.GetValue_Float(ki * 3 + 2) * -1;
						destAnimNode->m_translations.push_back(v);
					}
					else if (sampler.interpolation == "CUBICSPLINE")
					{
						v.m_vec.x = valueGetter.GetValue_Float(ki * 9 + 3);
						v.m_vec.y = valueGetter.GetValue_Float(ki * 9 + 4);
						v.m_vec.z = valueGetter.GetValue_Float(ki * 9 + 5) * -1;
						destAnimNode->m_translations.push_back(v);
					}
				}
			}
			else if (channel.target_path == "scale")
			{
				for (UINT ki = 0; ki < timeGetter.GetAccessor()->count; ki++)
				{
					KdAnimKeyVector3 v;
					// 時間
					v.m_time = timeGetter.GetValue_Float(ki) * 60.0f;	// 元が60fpsとして変換
					if (v.m_time > animation->m_maxLength)
					{
						animation->m_maxLength = v.m_time;
					}

					// 値
					if (sampler.interpolation == "STEP")
					{
						v.m_vec.x = valueGetter.GetValue_Float(ki * 3 + 0);
						v.m_vec.y = valueGetter.GetValue_Float(ki * 3 + 1);
						v.m_vec.z = valueGetter.GetValue_Float(ki * 3 + 2);
						destAnimNode->m_scales.push_back(v);
					}
					else if (sampler.interpolation == "LINEAR")
					{
						v.m_vec.x = valueGetter.GetValue_Float(ki * 3 + 0);
						v.m_vec.y = valueGetter.GetValue_Float(ki * 3 + 1);
						v.m_vec.z = valueGetter.GetValue_Float(ki * 3 + 2);
						destAnimNode->m_scales.push_back(v);
					}
					else if (sampler.interpolation == "CUBICSPLINE")
					{
						v.m_vec.x = valueGetter.GetValue_Float(ki * 9 + 3);
						v.m_vec.y = valueGetter.GetValue_Float(ki * 9 + 4);
						v.m_vec.z = valueGetter.GetValue_Float(ki * 9 + 5);
						destAnimNode->m_scales.push_back(v);
					}
				}
			}
			else if (channel.target_path == "rotation")
			{
				for (UINT ki = 0; ki < timeGetter.GetAccessor()->count; ki++)
				{
					KdAnimKeyQuaternion q;
					// 時間
					q.m_time = timeGetter.GetValue_Float(ki) * 60.0f;	// 元が60fpsとして変換
					if (q.m_time > animation->m_maxLength)
					{
						animation->m_maxLength = q.m_time;
					}

					if (sampler.interpolation == "STEP")
					{
						q.m_quat.y = valueGetter.GetValue_Float(ki * 4 + 1) * -1;
						q.m_quat.x = valueGetter.GetValue_Float(ki * 4 + 0) * -1;
						q.m_quat.z = valueGetter.GetValue_Float(ki * 4 + 2);
						q.m_quat.w = valueGetter.GetValue_Float(ki * 4 + 3);
						destAnimNode->m_rotations.push_back(q);
					}
					else if (sampler.interpolation == "LINEAR")
					{
						q.m_quat.x = valueGetter.GetValue_Float(ki * 4 + 0) * -1;
						q.m_quat.y = valueGetter.GetValue_Float(ki * 4 + 1) * -1;
						q.m_quat.z = valueGetter.GetValue_Float(ki * 4 + 2);
						q.m_quat.w = valueGetter.GetValue_Float(ki * 4 + 3);
						destAnimNode->m_rotations.push_back(q);
					}
					else if (sampler.interpolation == "CUBICSPLINE")
					{
						q.m_quat.x = valueGetter.GetValue_Float(ki * 12 + 4) * -1;
						q.m_quat.y = valueGetter.GetValue_Float(ki * 12 + 5) * -1;
						q.m_quat.z = valueGetter.GetValue_Float(ki * 12 + 6);
						q.m_quat.w = valueGetter.GetValue_Float(ki * 12 + 7);
						destAnimNode->m_rotations.push_back(q);
					}
				}
			}
		}

		// アニメーションで使用していない不必要なノードを除外したリスト作成
		for (auto&& n : tempNodes)
		{
			if (n == nullptr)continue;
			animation->m_nodes.push_back(n);
		}
	}

	return destModel;
}


//===================================================
//
// ここからはデバッグ表示用
//
//===================================================


#ifdef GLTF_DEBUG

#include <cstdio>
#include <fstream>
#include <iostream>

static std::string PrintMode(int mode) {
	if (mode == TINYGLTF_MODE_POINTS) {
		return "POINTS";
	}
	else if (mode == TINYGLTF_MODE_LINE) {
		return "LINE";
	}
	else if (mode == TINYGLTF_MODE_LINE_LOOP) {
		return "LINE_LOOP";
	}
	else if (mode == TINYGLTF_MODE_TRIANGLES) {
		return "TRIANGLES";
	}
	else if (mode == TINYGLTF_MODE_TRIANGLE_FAN) {
		return "TRIANGLE_FAN";
	}
	else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
		return "TRIANGLE_STRIP";
	}
	return "**UNKNOWN**";
}

static std::string PrintTarget(int target) {
	if (target == 34962) {
		return "GL_ARRAY_BUFFER";
	}
	else if (target == 34963) {
		return "GL_ELEMENT_ARRAY_BUFFER";
	}
	else {
		return "**UNKNOWN**";
	}
}

static std::string PrintType(int ty) {
	if (ty == TINYGLTF_TYPE_SCALAR) {
		return "SCALAR";
	}
	else if (ty == TINYGLTF_TYPE_VECTOR) {
		return "VECTOR";
	}
	else if (ty == TINYGLTF_TYPE_VEC2) {
		return "VEC2";
	}
	else if (ty == TINYGLTF_TYPE_VEC3) {
		return "VEC3";
	}
	else if (ty == TINYGLTF_TYPE_VEC4) {
		return "VEC4";
	}
	else if (ty == TINYGLTF_TYPE_MATRIX) {
		return "MATRIX";
	}
	else if (ty == TINYGLTF_TYPE_MAT2) {
		return "MAT2";
	}
	else if (ty == TINYGLTF_TYPE_MAT3) {
		return "MAT3";
	}
	else if (ty == TINYGLTF_TYPE_MAT4) {
		return "MAT4";
	}
	return "**UNKNOWN**";
}

static std::string PrintComponentType(int ty) {
	if (ty == TINYGLTF_COMPONENT_TYPE_BYTE) {
		return "BYTE";
	}
	else if (ty == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
		return "UNSIGNED_BYTE";
	}
	else if (ty == TINYGLTF_COMPONENT_TYPE_SHORT) {
		return "SHORT";
	}
	else if (ty == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
		return "UNSIGNED_SHORT";
	}
	else if (ty == TINYGLTF_COMPONENT_TYPE_INT) {
		return "INT";
	}
	else if (ty == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		return "UNSIGNED_INT";
	}
	else if (ty == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		return "FLOAT";
	}
	else if (ty == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
		return "DOUBLE";
	}

	return "**UNKNOWN**";
}

#if 0
static std::string PrintParameterType(int ty) {
	if (ty == TINYGLTF_PARAMETER_TYPE_BYTE) {
		return "BYTE";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE) {
		return "UNSIGNED_BYTE";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_SHORT) {
		return "SHORT";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) {
		return "UNSIGNED_SHORT";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_INT) {
		return "INT";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT) {
		return "UNSIGNED_INT";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT) {
		return "FLOAT";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC2) {
		return "FLOAT_VEC2";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3) {
		return "FLOAT_VEC3";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC4) {
		return "FLOAT_VEC4";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_INT_VEC2) {
		return "INT_VEC2";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_INT_VEC3) {
		return "INT_VEC3";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_INT_VEC4) {
		return "INT_VEC4";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_BOOL) {
		return "BOOL";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_BOOL_VEC2) {
		return "BOOL_VEC2";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_BOOL_VEC3) {
		return "BOOL_VEC3";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_BOOL_VEC4) {
		return "BOOL_VEC4";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_MAT2) {
		return "FLOAT_MAT2";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_MAT3) {
		return "FLOAT_MAT3";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_FLOAT_MAT4) {
		return "FLOAT_MAT4";
	}
	else if (ty == TINYGLTF_PARAMETER_TYPE_SAMPLER_2D) {
		return "SAMPLER_2D";
	}

	return "**UNKNOWN**";
}
#endif

static std::string PrintWrapMode(int mode) {
	if (mode == TINYGLTF_TEXTURE_WRAP_REPEAT) {
		return "REPEAT";
	}
	else if (mode == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE) {
		return "CLAMP_TO_EDGE";
	}
	else if (mode == TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT) {
		return "MIRRORED_REPEAT";
	}

	return "**UNKNOWN**";
}

static std::string PrintFilterMode(int mode) {
	if (mode == TINYGLTF_TEXTURE_FILTER_NEAREST) {
		return "NEAREST";
	}
	else if (mode == TINYGLTF_TEXTURE_FILTER_LINEAR) {
		return "LINEAR";
	}
	else if (mode == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST) {
		return "NEAREST_MIPMAP_NEAREST";
	}
	else if (mode == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR) {
		return "NEAREST_MIPMAP_LINEAR";
	}
	else if (mode == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST) {
		return "LINEAR_MIPMAP_NEAREST";
	}
	else if (mode == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR) {
		return "LINEAR_MIPMAP_LINEAR";
	}
	return "**UNKNOWN**";
}

static std::string PrintIntArray(const std::vector<int> &arr) {
	if (arr.size() == 0) {
		return "";
	}

	std::stringstream ss;
	ss << "[ ";
	for (size_t i = 0; i < arr.size(); i++) {
		ss << arr[i];
		if (i != arr.size() - 1) {
			ss << ", ";
		}
	}
	ss << " ]";

	return ss.str();
}

static std::string PrintFloatArray(const std::vector<double> &arr) {
	if (arr.size() == 0) {
		return "";
	}

	std::stringstream ss;
	ss << "[ ";
	for (size_t i = 0; i < arr.size(); i++) {
		ss << arr[i];
		if (i != arr.size() - 1) {
			ss << ", ";
		}
	}
	ss << " ]";

	return ss.str();
}

static std::string Indent(const int indent) {
	std::string s;
	for (int i = 0; i < indent; i++) {
		s += "  ";
	}

	return s;
}

static std::string PrintParameterValue(const tinygltf::Parameter &param) {
	if (!param.number_array.empty()) {
		return PrintFloatArray(param.number_array);
	}
	else {
		return param.string_value;
	}
}

#if 0
static std::string PrintParameterMap(const tinygltf::ParameterMap &pmap) {
	std::stringstream ss;

	ss << pmap.size() << std::endl;
	for (auto &kv : pmap) {
		ss << kv.first << " : " << PrintParameterValue(kv.second) << std::endl;
	}

	return ss.str();
}
#endif

static std::string PrintValue(const std::string &name,
	const tinygltf::Value &value, const int indent,
	const bool tag = true) {
	std::stringstream ss;

	if (value.IsObject()) {
		const tinygltf::Value::Object &o = value.Get<tinygltf::Value::Object>();
		tinygltf::Value::Object::const_iterator it(o.begin());
		tinygltf::Value::Object::const_iterator itEnd(o.end());
		for (; it != itEnd; it++) {
			ss << PrintValue(it->first, it->second, indent + 1) << std::endl;
		}
	}
	else if (value.IsString()) {
		if (tag) {
			ss << Indent(indent) << name << " : " << value.Get<std::string>();
		}
		else {
			ss << Indent(indent) << value.Get<std::string>() << " ";
		}
	}
	else if (value.IsBool()) {
		if (tag) {
			ss << Indent(indent) << name << " : " << value.Get<bool>();
		}
		else {
			ss << Indent(indent) << value.Get<bool>() << " ";
		}
	}
	else if (value.IsNumber()) {
		if (tag) {
			ss << Indent(indent) << name << " : " << value.Get<double>();
		}
		else {
			ss << Indent(indent) << value.Get<double>() << " ";
		}
	}
	else if (value.IsInt()) {
		if (tag) {
			ss << Indent(indent) << name << " : " << value.Get<int>();
		}
		else {
			ss << Indent(indent) << value.Get<int>() << " ";
		}
	}
	else if (value.IsArray()) {
		// TODO(syoyo): Better pretty printing of array item
		ss << Indent(indent) << name << " [ \n";
		for (size_t i = 0; i < value.Size(); i++) {
			ss << PrintValue("", value.Get(int(i)), indent + 1, /* tag */ false);
			if (i != (value.ArrayLen() - 1)) {
				ss << ", \n";
			}
		}
		ss << "\n" << Indent(indent) << "] ";
	}

	// @todo { binary }

	return ss.str();
}

static void DumpNode(const tinygltf::Node &node, int indent) {
	std::cout << Indent(indent) << "name        : " << node.name << std::endl;
	std::cout << Indent(indent) << "camera      : " << node.camera << std::endl;
	std::cout << Indent(indent) << "mesh        : " << node.mesh << std::endl;
	if (!node.rotation.empty()) {
		std::cout << Indent(indent)
			<< "rotation    : " << PrintFloatArray(node.rotation)
			<< std::endl;
	}
	if (!node.scale.empty()) {
		std::cout << Indent(indent)
			<< "scale       : " << PrintFloatArray(node.scale) << std::endl;
	}
	if (!node.translation.empty()) {
		std::cout << Indent(indent)
			<< "translation : " << PrintFloatArray(node.translation)
			<< std::endl;
	}

	if (!node.matrix.empty()) {
		std::cout << Indent(indent)
			<< "matrix      : " << PrintFloatArray(node.matrix) << std::endl;
	}

	std::cout << Indent(indent)
		<< "children    : " << PrintIntArray(node.children) << std::endl;
}

static void DumpStringIntMap(const std::map<std::string, int> &m, int indent) {
	std::map<std::string, int>::const_iterator it(m.begin());
	std::map<std::string, int>::const_iterator itEnd(m.end());
	for (; it != itEnd; it++) {
		std::cout << Indent(indent) << it->first << ": " << it->second << std::endl;
	}
}

static void DumpPrimitive(const tinygltf::Primitive &primitive, int indent) {
	std::cout << Indent(indent) << "material : " << primitive.material
		<< std::endl;
	std::cout << Indent(indent) << "indices : " << primitive.indices << std::endl;
	std::cout << Indent(indent) << "mode     : " << PrintMode(primitive.mode)
		<< "(" << primitive.mode << ")" << std::endl;
	std::cout << Indent(indent)
		<< "attributes(items=" << primitive.attributes.size() << ")"
		<< std::endl;
	DumpStringIntMap(primitive.attributes, indent + 1);

	std::cout << Indent(indent) << "extras :" << std::endl
		<< PrintValue("extras", primitive.extras, indent + 1) << std::endl;
}

static void DumpExtensions(const tinygltf::ExtensionMap &extension,
	const int indent) {
	// TODO(syoyo): pritty print Value
	for (auto &e : extension) {
		std::cout << Indent(indent) << e.first << std::endl;
		std::cout << PrintValue("extensions", e.second, indent + 1) << std::endl;
	}
}

static void DumpTextureInfo(const tinygltf::TextureInfo &texinfo,
	const int indent) {
	std::cout << Indent(indent) << "index     : " << texinfo.index << "\n";
	std::cout << Indent(indent) << "texCoord  : TEXCOORD_" << texinfo.texCoord
		<< "\n";
	DumpExtensions(texinfo.extensions, indent + 1);
	std::cout << PrintValue("extras", texinfo.extras, indent + 1) << "\n";
}

static void DumpNormalTextureInfo(const tinygltf::NormalTextureInfo &texinfo,
	const int indent) {
	std::cout << Indent(indent) << "index     : " << texinfo.index << "\n";
	std::cout << Indent(indent) << "texCoord  : TEXCOORD_" << texinfo.texCoord
		<< "\n";
	std::cout << Indent(indent) << "scale     : " << texinfo.scale << "\n";
	DumpExtensions(texinfo.extensions, indent + 1);
	std::cout << PrintValue("extras", texinfo.extras, indent + 1) << "\n";
}

static void DumpOcclusionTextureInfo(
	const tinygltf::OcclusionTextureInfo &texinfo, const int indent) {
	std::cout << Indent(indent) << "index     : " << texinfo.index << "\n";
	std::cout << Indent(indent) << "texCoord  : TEXCOORD_" << texinfo.texCoord
		<< "\n";
	std::cout << Indent(indent) << "strength  : " << texinfo.strength << "\n";
	DumpExtensions(texinfo.extensions, indent + 1);
	std::cout << PrintValue("extras", texinfo.extras, indent + 1) << "\n";
}

static void DumpPbrMetallicRoughness(const tinygltf::PbrMetallicRoughness &pbr,
	const int indent) {
	std::cout << Indent(indent)
		<< "baseColorFactor   : " << PrintFloatArray(pbr.baseColorFactor)
		<< "\n";
	std::cout << Indent(indent) << "baseColorTexture  :\n";
	DumpTextureInfo(pbr.baseColorTexture, indent + 1);

	std::cout << Indent(indent) << "metallicFactor    : " << pbr.metallicFactor
		<< "\n";
	std::cout << Indent(indent) << "roughnessFactor   : " << pbr.roughnessFactor
		<< "\n";

	std::cout << Indent(indent) << "metallicRoughnessTexture  :\n";
	DumpTextureInfo(pbr.metallicRoughnessTexture, indent + 1);
	DumpExtensions(pbr.extensions, indent + 1);
	std::cout << PrintValue("extras", pbr.extras, indent + 1) << "\n";
}

static void Dump(const tinygltf::Model &model) {
	std::cout << "=== Dump glTF ===" << std::endl;
	std::cout << "asset.copyright          : " << model.asset.copyright
		<< std::endl;
	std::cout << "asset.generator          : " << model.asset.generator
		<< std::endl;
	std::cout << "asset.version            : " << model.asset.version
		<< std::endl;
	std::cout << "asset.minVersion         : " << model.asset.minVersion
		<< std::endl;
	std::cout << std::endl;

	std::cout << "=== Dump scene ===" << std::endl;
	std::cout << "defaultScene: " << model.defaultScene << std::endl;

	{
		std::cout << "scenes(items=" << model.scenes.size() << ")" << std::endl;
		for (size_t i = 0; i < model.scenes.size(); i++) {
			std::cout << Indent(1) << "scene[" << i
				<< "] name  : " << model.scenes[i].name << std::endl;
			DumpExtensions(model.scenes[i].extensions, 1);
		}
	}

	{
		std::cout << "meshes(item=" << model.meshes.size() << ")" << std::endl;
		for (size_t i = 0; i < model.meshes.size(); i++) {
			std::cout << Indent(1) << "name     : " << model.meshes[i].name
				<< std::endl;
			std::cout << Indent(1)
				<< "primitives(items=" << model.meshes[i].primitives.size()
				<< "): " << std::endl;

			for (size_t k = 0; k < model.meshes[i].primitives.size(); k++) {
				DumpPrimitive(model.meshes[i].primitives[k], 2);
			}
		}
	}

	{
		for (size_t i = 0; i < model.accessors.size(); i++) {
			const tinygltf::Accessor &accessor = model.accessors[i];
			std::cout << Indent(1) << "name         : " << accessor.name << std::endl;
			std::cout << Indent(2) << "bufferView   : " << accessor.bufferView
				<< std::endl;
			std::cout << Indent(2) << "byteOffset   : " << accessor.byteOffset
				<< std::endl;
			std::cout << Indent(2) << "componentType: "
				<< PrintComponentType(accessor.componentType) << "("
				<< accessor.componentType << ")" << std::endl;
			std::cout << Indent(2) << "count        : " << accessor.count
				<< std::endl;
			std::cout << Indent(2) << "type         : " << PrintType(accessor.type)
				<< std::endl;
			if (!accessor.minValues.empty()) {
				std::cout << Indent(2) << "min          : [";
				for (size_t k = 0; k < accessor.minValues.size(); k++) {
					std::cout << accessor.minValues[k]
						<< ((k != accessor.minValues.size() - 1) ? ", " : "");
				}
				std::cout << "]" << std::endl;
			}
			if (!accessor.maxValues.empty()) {
				std::cout << Indent(2) << "max          : [";
				for (size_t k = 0; k < accessor.maxValues.size(); k++) {
					std::cout << accessor.maxValues[k]
						<< ((k != accessor.maxValues.size() - 1) ? ", " : "");
				}
				std::cout << "]" << std::endl;
			}

			if (accessor.sparse.isSparse) {
				std::cout << Indent(2) << "sparse:" << std::endl;
				std::cout << Indent(3) << "count  : " << accessor.sparse.count
					<< std::endl;
				std::cout << Indent(3) << "indices: " << std::endl;
				std::cout << Indent(4)
					<< "bufferView   : " << accessor.sparse.indices.bufferView
					<< std::endl;
				std::cout << Indent(4)
					<< "byteOffset   : " << accessor.sparse.indices.byteOffset
					<< std::endl;
				std::cout << Indent(4) << "componentType: "
					<< PrintComponentType(accessor.sparse.indices.componentType)
					<< "(" << accessor.sparse.indices.componentType << ")"
					<< std::endl;
				std::cout << Indent(3) << "values : " << std::endl;
				std::cout << Indent(4)
					<< "bufferView   : " << accessor.sparse.values.bufferView
					<< std::endl;
				std::cout << Indent(4)
					<< "byteOffset   : " << accessor.sparse.values.byteOffset
					<< std::endl;
			}
		}
	}

	{
		std::cout << "animations(items=" << model.animations.size() << ")"
			<< std::endl;
		for (size_t i = 0; i < model.animations.size(); i++) {
			const tinygltf::Animation &animation = model.animations[i];
			std::cout << Indent(1) << "name         : " << animation.name
				<< std::endl;

			std::cout << Indent(1) << "channels : [ " << std::endl;
			for (size_t j = 0; i < animation.channels.size(); i++) {
				std::cout << Indent(2)
					<< "sampler     : " << animation.channels[j].sampler
					<< std::endl;
				std::cout << Indent(2)
					<< "target.id   : " << animation.channels[j].target_node
					<< std::endl;
				std::cout << Indent(2)
					<< "target.path : " << animation.channels[j].target_path
					<< std::endl;
				std::cout << ((i != (animation.channels.size() - 1)) ? "  , " : "");
			}
			std::cout << "  ]" << std::endl;

			std::cout << Indent(1) << "samplers(items=" << animation.samplers.size()
				<< ")" << std::endl;
			for (size_t j = 0; j < animation.samplers.size(); j++) {
				const tinygltf::AnimationSampler &sampler = animation.samplers[j];
				std::cout << Indent(2) << "input         : " << sampler.input
					<< std::endl;
				std::cout << Indent(2) << "interpolation : " << sampler.interpolation
					<< std::endl;
				std::cout << Indent(2) << "output        : " << sampler.output
					<< std::endl;
			}
		}
	}

	{
		std::cout << "bufferViews(items=" << model.bufferViews.size() << ")"
			<< std::endl;
		for (size_t i = 0; i < model.bufferViews.size(); i++) {
			const tinygltf::BufferView &bufferView = model.bufferViews[i];
			std::cout << Indent(1) << "name         : " << bufferView.name
				<< std::endl;
			std::cout << Indent(2) << "buffer       : " << bufferView.buffer
				<< std::endl;
			std::cout << Indent(2) << "byteLength   : " << bufferView.byteLength
				<< std::endl;
			std::cout << Indent(2) << "byteOffset   : " << bufferView.byteOffset
				<< std::endl;
			std::cout << Indent(2) << "byteStride   : " << bufferView.byteStride
				<< std::endl;
			std::cout << Indent(2)
				<< "target       : " << PrintTarget(bufferView.target)
				<< std::endl;
		}
	}

	{
		std::cout << "buffers(items=" << model.buffers.size() << ")" << std::endl;
		for (size_t i = 0; i < model.buffers.size(); i++) {
			const tinygltf::Buffer &buffer = model.buffers[i];
			std::cout << Indent(1) << "name         : " << buffer.name << std::endl;
			std::cout << Indent(2) << "byteLength   : " << buffer.data.size()
				<< std::endl;
		}
	}

	{
		std::cout << "materials(items=" << model.materials.size() << ")"
			<< std::endl;
		for (size_t i = 0; i < model.materials.size(); i++) {
			const tinygltf::Material &material = model.materials[i];
			std::cout << Indent(1) << "name                 : " << material.name
				<< std::endl;

			std::cout << Indent(1) << "alphaMode            : " << material.alphaMode
				<< std::endl;
			std::cout << Indent(1)
				<< "alphaCutoff          : " << material.alphaCutoff
				<< std::endl;
			std::cout << Indent(1) << "doubleSided          : "
				<< (material.doubleSided ? "true" : "false") << std::endl;
			std::cout << Indent(1) << "emissiveFactor       : "
				<< PrintFloatArray(material.emissiveFactor) << std::endl;

			std::cout << Indent(1) << "pbrMetallicRoughness :\n";
			DumpPbrMetallicRoughness(material.pbrMetallicRoughness, 2);

			std::cout << Indent(1) << "normalTexture        :\n";
			DumpNormalTextureInfo(material.normalTexture, 2);

			std::cout << Indent(1) << "occlusionTexture     :\n";
			DumpOcclusionTextureInfo(material.occlusionTexture, 2);

			std::cout << Indent(1) << "emissiveTexture      :\n";
			DumpTextureInfo(material.emissiveTexture, 2);

			std::cout << Indent(1) << "----  legacy material parameter  ----\n";
			std::cout << Indent(1) << "values(items=" << material.values.size() << ")"
				<< std::endl;
			tinygltf::ParameterMap::const_iterator p(material.values.begin());
			tinygltf::ParameterMap::const_iterator pEnd(material.values.end());
			for (; p != pEnd; p++) {
				std::cout << Indent(2) << p->first << ": "
					<< PrintParameterValue(p->second) << std::endl;
			}
			std::cout << Indent(1) << "-------------------------------------\n";

			DumpExtensions(material.extensions, 1);
			std::cout << PrintValue("extras", material.extras, 2) << std::endl;
		}
	}

	{
		std::cout << "nodes(items=" << model.nodes.size() << ")" << std::endl;
		for (size_t i = 0; i < model.nodes.size(); i++) {
			const tinygltf::Node &node = model.nodes[i];
			std::cout << Indent(1) << "name         : " << node.name << std::endl;

			DumpNode(node, 2);
		}
	}

	{
		std::cout << "images(items=" << model.images.size() << ")" << std::endl;
		for (size_t i = 0; i < model.images.size(); i++) {
			const tinygltf::Image &image = model.images[i];
			std::cout << Indent(1) << "name         : " << image.name << std::endl;

			std::cout << Indent(2) << "width     : " << image.width << std::endl;
			std::cout << Indent(2) << "height    : " << image.height << std::endl;
			std::cout << Indent(2) << "component : " << image.component << std::endl;
			DumpExtensions(image.extensions, 1);
		}
	}

	{
		std::cout << "textures(items=" << model.textures.size() << ")" << std::endl;
		for (size_t i = 0; i < model.textures.size(); i++) {
			const tinygltf::Texture &texture = model.textures[i];
			std::cout << Indent(1) << "sampler        : " << texture.sampler
				<< std::endl;
			std::cout << Indent(1) << "source         : " << texture.source
				<< std::endl;
			DumpExtensions(texture.extensions, 1);
		}
	}

	{
		std::cout << "samplers(items=" << model.samplers.size() << ")" << std::endl;

		for (size_t i = 0; i < model.samplers.size(); i++) {
			const tinygltf::Sampler &sampler = model.samplers[i];
			std::cout << Indent(1) << "name (id)    : " << sampler.name << std::endl;
			std::cout << Indent(2)
				<< "minFilter    : " << PrintFilterMode(sampler.minFilter)
				<< std::endl;
			std::cout << Indent(2)
				<< "magFilter    : " << PrintFilterMode(sampler.magFilter)
				<< std::endl;
			std::cout << Indent(2)
				<< "wrapS        : " << PrintWrapMode(sampler.wrapS)
				<< std::endl;
			std::cout << Indent(2)
				<< "wrapT        : " << PrintWrapMode(sampler.wrapT)
				<< std::endl;
		}
	}

	{
		std::cout << "cameras(items=" << model.cameras.size() << ")" << std::endl;

		for (size_t i = 0; i < model.cameras.size(); i++) {
			const tinygltf::Camera &camera = model.cameras[i];
			std::cout << Indent(1) << "name (id)    : " << camera.name << std::endl;
			std::cout << Indent(1) << "type         : " << camera.type << std::endl;

			if (camera.type.compare("perspective") == 0) {
				std::cout << Indent(2)
					<< "aspectRatio   : " << camera.perspective.aspectRatio
					<< std::endl;
				std::cout << Indent(2) << "yfov          : " << camera.perspective.yfov
					<< std::endl;
				std::cout << Indent(2) << "zfar          : " << camera.perspective.zfar
					<< std::endl;
				std::cout << Indent(2) << "znear         : " << camera.perspective.znear
					<< std::endl;
			}
			else if (camera.type.compare("orthographic") == 0) {
				std::cout << Indent(2) << "xmag          : " << camera.orthographic.xmag
					<< std::endl;
				std::cout << Indent(2) << "ymag          : " << camera.orthographic.ymag
					<< std::endl;
				std::cout << Indent(2) << "zfar          : " << camera.orthographic.zfar
					<< std::endl;
				std::cout << Indent(2)
					<< "znear         : " << camera.orthographic.znear
					<< std::endl;
			}
		}
	}

	// toplevel extensions
	{
		std::cout << "extensions(items=" << model.extensions.size() << ")"
			<< std::endl;
		DumpExtensions(model.extensions, 1);
	}
}

#endif


