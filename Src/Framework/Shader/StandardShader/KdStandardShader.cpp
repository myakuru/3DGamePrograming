#include "Framework/KdFramework.h"

#include "KdStandardShader.h"


//================================================
// 描画準備
//================================================

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 陰影をつけるオブジェクトの描画の直前処理（不透明な物体やキャラクタの板ポリゴン）
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// シェーダーのパイプライン変更
// LitShaderで使用するリソースのバッファー設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::BeginLit()
{
	// 頂点シェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetVertexShader(m_VS_Lit))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	// ピクセルシェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS_Lit))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
	}

	// CSM変更点
	// シャドウマップのテクスチャをセット
	ID3D11ShaderResourceView* srvs[Cascade::Num] =
	{
		m_cascadeShadowMapRTPack[0].m_RTTexture->WorkSRView(),
		m_cascadeShadowMapRTPack[1].m_RTTexture->WorkSRView(),
		m_cascadeShadowMapRTPack[2].m_RTTexture->WorkSRView()
	};
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(10, Cascade::Num, srvs);

	// ボーン情報をセット(スキンメッシュ対応)
	KdShaderManager::Instance().SetVSConstantBuffer(3, m_cb3_Bone.GetAddress());

	// 通常テクスチャ用サンプラーのセット
	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Anisotropic_Wrap, 0);

	// 影ぼかし用の比較機能付きサンプラーのセット
	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Linear_Clamp_Cmp, 1);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 陰影ありオブジェクトの描画修了
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// 影を書き込む用に使用していたGenDepthFromLightで生成した深度SRVの解放
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::EndLit()
{
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(10, 1, &pNullSRV);
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(11, 1, &pNullSRV);
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(12, 1, &pNullSRV);

}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 陰影をつけないオブジェクトの描画の直前処理（エフェクトや半透明物）
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// シェーダーのパイプライン変更
// UnLitShaderで使用するリソースのバッファー設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::BeginUnLit()
{
	if (KdShaderManager::Instance().SetVertexShader(m_VS_UnLit))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	if (KdShaderManager::Instance().SetPixelShader(m_PS_UnLit))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 陰影なしオブジェクトの描画終了
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::EndUnLit()
{
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 影を生み出すオブジェクトの情報描画（光を遮る物体）
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// シェーダーのパイプライン変更
// GenDepthMapFromLightShaderで使用するリソースのバッファー設定
// 書き込むテクスチャーを深度用の赤一色のテクスチャに切り替え
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::BeginGenerateDepthMapFromLight()
{
	if (KdShaderManager::Instance().SetVertexShader(m_VS_GenDepthFromLight))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	// ボーン情報をセット(スキンメッシュ対応)
	KdShaderManager::Instance().SetVSConstantBuffer(3, m_cb3_Bone.GetAddress());

	if (KdShaderManager::Instance().SetPixelShader(m_PS_GenDepthFromLight))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
	}

	for (int i = 0; i < Cascade::Num; ++i)
	{
		m_cascadeShadowMapRTPack[i].ClearTexture(kRedColor);
	}

	m_isShadowPass = true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 影を生み出すオブジェクトの描画終了
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::EndGenerateDepthMapFromLight()
{
	m_isShadowPass = false;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// Toonシェーダーの描画準備
void KdStandardShader::BeginToon()
{
	// 頂点シェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetVertexShader(m_VS_Lit))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	// ピクセルシェーダーのパイプライン変更
	// トゥーンシェーダーに変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS_Toon))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
	}
}

// Toonシェーダーの描画終了
void KdStandardShader::EndToon()
{
}

void KdStandardShader::BeginGradient()
{
	// 頂点シェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetVertexShader(m_VS_Lit))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	// ピクセルシェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS_Gradation))
	{
		// ここでグラデーション情報をセット
		m_cb0_Obj.Work().enableGradient = m_enableGradient ? 1 : 0;
		m_cb0_Obj.Work().gradientColor = m_gradientColor;
		m_cb0_Obj.Write();
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
	}
}

void KdStandardShader::EndGradient()
{
}

void KdStandardShader::BeginGrayscale()
{
	// 頂点シェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetVertexShader(m_VS_Lit))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	// ピクセルシェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS_GrayScale))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
	}
}

void KdStandardShader::EndGrayscale()
{
}

void KdStandardShader::BeginEffect()
{
	// ピクセルシェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS_Effect))
	{
		m_cb4_Effect.Write();
		// b0にエフェクト用定数バッファをセット
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		// b2にマテリアル用定数バッファをセット
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
		// b4にエフェクト用定数バッファをセット
		KdShaderManager::Instance().SetPSConstantBuffer(4, m_cb4_Effect.GetAddress());
	}
}

void KdStandardShader::EndEffect()
{
}

void KdStandardShader::BeginRimLight()
{
	// 頂点シェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetVertexShader(m_VS_Lit))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	// ピクセルシェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS_RimLight))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
	}

}

void KdStandardShader::EndRimLight()
{
}


//================================================
// 描画関数
//================================================

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// メッシュを描画
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// メッシュの頂点データや3Dワールド情報・マテリアル情報をシェーダー(GPU)に転送する
// サブセットごとに描画命令を呼び出す：サブセットの個数分処理が重くなる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::DrawMesh(const KdMesh* mesh, const Math::Matrix& mWorld,
	const std::vector<KdMaterial>& materials, const Math::Vector4& colRate, const Math::Vector3& emissive)
{
	if (mesh == nullptr) { return; }

	// 視錐台カリング：影パス中はスキップ
	if (!m_isShadowPass)
	{
		const KdShaderManager::cbCamera* cbCam = &KdShaderManager::Instance().GetCameraCB();
		if (cbCam)
		{
			Math::Matrix _mView = KdShaderManager::Instance().GetCameraCB().mView;
			Math::Matrix _mProj = KdShaderManager::Instance().GetCameraCB().mProj;

			DirectX::BoundingOrientedBox obb;
			DirectX::BoundingOrientedBox::CreateFromBoundingBox(obb, mesh->GetBoundingBox());
			obb.Transform(obb, mWorld);

			DirectX::BoundingFrustum vf;
			DirectX::BoundingFrustum::CreateFromMatrix(vf, _mProj);

			Math::Matrix _mCam = _mView.Invert();
			vf.Origin = _mCam.Translation();
			vf.Orientation = Math::Quaternion::CreateFromRotationMatrix(_mCam);

			if (!vf.Intersects(obb)) return;
		}
	}

	// メッシュの頂点情報転送
	mesh->SetToDevice();

	// GenDepthFromLightシェーダーの場合は、カスケードシャドウマップ用に複数回描画を行う
	if (m_isShadowPass)
	{
		// 後面カリングなしにする
		KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullNone);

		// メッシュからバウンディングボックスを取得
		DirectX::BoundingBox _meshBox;
		mesh->GetBoundingBox().Transform(_meshBox, mWorld);

		// 描画レンダーのインデックス計算
		CascadeShadowMapChangea(_meshBox);

		for (int i = 0; i < Cascade::Num; i++)
		{
			// 対象のカスケードシャドウマップに描画が必要ない場合はスキップ
			if ((m_cascadeCount & (1 << i)) == 0) continue;

			// ローカルチェンジャーを使って切替（チェンジャーは Change->Undo のペアで使う）
			KdRenderTargetChanger rtChanger;
			if (!rtChanger.ChangeRenderTarget(m_cascadeShadowMapRTPack[i]))
			{
				continue;
			}

			// ビューポートや深度バッファも RTPack に入っているなら ChangeRenderTarget がセットするはず
			// 3Dワールド行列転送
			m_cb1_Mesh.Work().mW = mWorld;
			m_cb1_Mesh.Work().CascadeCount = i;
			m_cb1_Mesh.Write();

			// 全サブセット描画...
			for (UINT subi = 0; subi < mesh->GetSubsets().size(); subi++)
			{
				if (mesh->GetSubsets()[subi].FaceCount == 0) continue;
				const KdMaterial& material = materials[mesh->GetSubsets()[subi].MaterialNo];
				WriteMaterial(material, colRate, emissive);
				mesh->DrawSubset(subi);
			}

			// 元に戻す
			rtChanger.UndoRenderTarget();

		}

		KdShaderManager::Instance().UndoRasterizerState();
	}
	else
	{
		// 3Dワールド行列転送
		m_cb1_Mesh.Work().mW = mWorld;
		m_cb1_Mesh.Write();

		// 全サブセット
		for (UINT subi = 0; subi < mesh->GetSubsets().size(); subi++)
		{
			// 面が１枚も無い場合はスキップ
			if (mesh->GetSubsets()[subi].FaceCount == 0)continue;

			// マテリアルデータの転送
			const KdMaterial& material = materials[mesh->GetSubsets()[subi].MaterialNo];
			WriteMaterial(material, colRate, emissive);

			//-----------------------
			// サブセット描画
			//-----------------------
			mesh->DrawSubset(subi);
		}
	}

}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// モデルデータを描画（スタティック(アニメーションをしない)なモデル専用
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// データに所属する全ての描画用メッシュを描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::DrawModel(const KdModelData& rModel, const Math::Matrix& mWorld,
	const Math::Color& colRate, const Math::Vector3& emissive)
{
	// オブジェクト単位の情報転送
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	auto& dataNodes = rModel.GetOriginalNodes();

	// 全描画用メッシュノードを描画
	for (auto& nodeIdx : rModel.GetDrawMeshNodeIndices())
	{
		// 描画
		DrawMesh(dataNodes[nodeIdx].m_spMesh.get(), dataNodes[nodeIdx].m_worldTransform * mWorld,
			rModel.GetMaterials(), colRate, emissive);
	}

	// 定数に変更があった場合は自動的に初期状態に戻す
	if(m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// モデルワークを描画（ダイナミック(アニメーションをしない)なモデルに対応
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// データに所属する全ての描画用メッシュをワークの3D行列に従って描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::DrawModel(KdModelWork& rModel, const Math::Matrix& mWorld,
	const Math::Color& colRate, const Math::Vector3& emissive)
{
	if (!rModel.IsEnable()) { return; }

	const std::shared_ptr<KdModelData>& data = rModel.GetData();

	// データがないときはスキップ
	if (data == nullptr) { return; }

	if (rModel.NeedCalcNodeMatrices())
	{
		rModel.CalcNodeMatrices();
	}

	// オブジェクト単位の情報転送(スキンメッシュ対応)
	SetIsSkinMeshObj(data->IsSkinMesh());
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	auto& workNodes = rModel.GetNodes();
	auto& dataNodes = data->GetOriginalNodes();

	// スキンメッシュモデルの場合：ボーン情報を書き込み(スキンメッシュ対応)
	if (data->IsSkinMesh())
	{
		// ノード内からボーン情報を取得
		for (auto&& nodeIdx : data->GetBoneNodeIndices())
		{
			if (nodeIdx >= KdStandardShader::maxBoneBufferSize) { assert(0 && "転送できるボーンの上限数を超えました"); return; }

			auto& dataNode = dataNodes[nodeIdx];
			auto& workNode = workNodes[nodeIdx];

			// ボーン情報からGPUに渡す行列の計算
			m_cb3_Bone.Work().mBones[dataNode.m_boneIndex] = dataNode.m_boneInverseWorldMatrix * workNode.m_worldTransform;

			m_cb3_Bone.Write();
		}
	}

	// 全描画用メッシュノードを描画
	for (auto& nodeIdx : data->GetDrawMeshNodeIndices())
	{
		// 描画
		DrawMesh(dataNodes[nodeIdx].m_spMesh.get(), workNodes[nodeIdx].m_worldTransform * mWorld,
			data->GetMaterials(), colRate, emissive);
	}

	// 定数に変更があった場合は自動的に初期状態に戻す
	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ポリゴンを描画（モデル以外のプログラム上で生成された頂点の集合体
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// データに所属する全ての描画用メッシュをワークの3D行列に従って描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::DrawPolygon(const KdPolygon& rPolygon, const Math::Matrix& mWorld,
	const Math::Color& colRate, const Math::Vector3& emissive)
{
	if (!rPolygon.IsEnable()) { return; }

	// ポリゴン描画用の頂点取得
	auto& vertices = rPolygon.GetVertices();

	// 頂点数が3より少なければポリゴンが形成できないので描画不能
	if (vertices.size() < 3) { return; }

	// オブジェクト単位の定数バッファで変更があった場合のみ情報転送
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	// 3Dワールド行列転送
	m_cb1_Mesh.Work().mW = mWorld;
	m_cb1_Mesh.Write();

	// マテリアルの転送
	if (rPolygon.GetMaterial())
	{
		WriteMaterial(*rPolygon.GetMaterial(), colRate, emissive);
	}
	else
	{
		WriteMaterial(KdMaterial(), colRate, emissive);
	}

	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullNone);

	// サンプラーステートの変更:ポリゴンの描画なので、テクスチャの末端が繰り返されると不自然な描画になるため変更が必要
	if (KdShaderManager::Instance().IsPixelArtStyle())
	{
		KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Point_Clamp);
	}
	else
	{
		KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Anisotropic_Clamp);
	}

	// 描画パイプラインのチェック
	ID3D11VertexShader* pNowVS = nullptr;
	KdDirect3D::Instance().WorkDevContext()->VSGetShader(&pNowVS, nullptr, nullptr);
	bool isLitShader = m_VS_Lit == pNowVS;
	KdSafeRelease(pNowVS);

	// 陰影ありのシェーダーで2Dオブジェクトを描画する時
	if (isLitShader && rPolygon.Is2DObject())
	{
		std::vector<KdPolygon::Vertex> _2DVertices = vertices;

		// ポリゴンの法線を光に向ける処理：どの方向に向いていても光の影響を正面からに受けるように変換
		ConvertNormalsFor2D(_2DVertices, mWorld);

		// 2DObject用に変換した頂点配列を描画
		KdDirect3D::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, (signed)_2DVertices.size(), &_2DVertices[0], sizeof(KdPolygon::Vertex));
	}
	else
	{
		// 頂点配列を描画
		KdDirect3D::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, (signed)vertices.size(), &vertices[0], sizeof(KdPolygon::Vertex));
	}

	KdShaderManager::Instance().UndoSamplerState();

	KdShaderManager::Instance().UndoRasterizerState();

	// 定数に変更があった場合は自動的に初期状態に戻す
	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

void KdStandardShader::DrawVertices(const std::vector<KdPolygon::Vertex>& vertices, const Math::Matrix& mWorld,
	const Math::Color& colRate)
{
	// 頂点数が2より少なければポリゴンが形成できないので描画不能
	if (vertices.size() < 2) { return; }

	// オブジェクト単位の定数バッファで変更があった場合のみ情報転送
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	// 3Dワールド行列転送
	m_cb1_Mesh.Work().mW = mWorld;
	m_cb1_Mesh.Write();

	// マテリアルの転送
	WriteMaterial(KdMaterial(), colRate, Math::Vector3::Zero);

	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullNone);
	KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZDisable);

	// サンプラーステートの変更:ポリゴンの描画なので、テクスチャの末端が繰り返されると不自然な描画になるため変更が必要
	if (KdShaderManager::Instance().IsPixelArtStyle())
	{
		KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Point_Clamp);
	}
	else
	{
		KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Anisotropic_Clamp);
	}

	// 描画パイプラインのチェック
	ID3D11VertexShader* pNowVS = nullptr;
	KdDirect3D::Instance().WorkDevContext()->VSGetShader(&pNowVS, nullptr, nullptr);

	KdSafeRelease(pNowVS);

	// 頂点配列を描画
	KdDirect3D::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINELIST, (signed)vertices.size(), &vertices[0], sizeof(KdPolygon::Vertex));

	KdShaderManager::Instance().UndoSamplerState();

	KdShaderManager::Instance().UndoDepthStencilState();

	KdShaderManager::Instance().UndoRasterizerState();
	// 定数に変更があった場合は自動的に初期状態に戻す
	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// KdShaderManagerの初期化時に呼び出される
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// シェーダー本体の生成
// シェーダーで利用する定数バッファの生成
// 影用の光からの深度情報テクスチャを生成
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdStandardShader::Init()
{
	//-------------------------------------
	// 頂点シェーダ(スキンメッシュ対応)
	//-------------------------------------
	{
		// コンパイル済みのシェーダーヘッダーファイルをインクルード
#include "KdStandardShader_VS_Lit.shaderInc"

		// 頂点シェーダー作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS_Lit))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}

		// １頂点の詳細な情報
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SKININDEX",	0, DXGI_FORMAT_R16G16B16A16_UINT,	0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SKINWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 56,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		// 頂点入力レイアウト作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateInputLayout(
			&layout[0],				// 入力エレメント先頭アドレス
			layout.size(),			// 入力エレメント数
			&compiledBuffer[0],		// 頂点バッファのバイナリデータ
			sizeof(compiledBuffer),	// 上記のバッファサイズ
			&m_inputLayout))
			) {
			assert(0 && "CreateInputLayout失敗");
			Release();
			return false;
		}
	}

	{
#include "KdStandardShader_VS_GenDepthMapFromLight.shaderInc"

		// 頂点シェーダー作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS_GenDepthFromLight))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}
	}

	{
#include "KdStandardShader_VS_UnLit.shaderInc"

		// 頂点シェーダー作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS_UnLit))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}
	}

	//-------------------------------------
	// ピクセルシェーダ
	//-------------------------------------
	{
#include "KdStandardShader_PS_Lit.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_Lit))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}

	{
#include "KdStandardShader_PS_GenDepthMapFromLight.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_GenDepthFromLight))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}

	{
#include "KdStandardShader_PS_UnLit.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_UnLit))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}

	{
#include"PS_Toon.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_Toon))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}

	{
#include"PS_Gradation.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_Gradation))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}
	{
#include"PS_GrayScale.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_GrayScale))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}
	{
#include"PS_Effect.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_Effect))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}
	{
#include"PS_RimLight.shaderInc"
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_RimLight))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}

	//-------------------------------------
	// 定数バッファ作成
	//-------------------------------------
	m_cb0_Obj.Create();
	m_cb1_Mesh.Create();
	m_cb2_Material.Create();

	// スキンメッシュ対応
	m_cb3_Bone.Create();

	// エフェクト用
	m_cb4_Effect.Create();

	// CSM用の深度マップ作成
	for (int i = 0; i < Cascade::Num; i++)
	{
		int ShadowMapExp = Cascade::MaxExp - i; // 12 - 0 ~ 12 - 3
		int shadowSize = 1 << ShadowMapExp;	// 影のサイズ計算
		int shadowWidth = shadowSize; // 横幅
		int shadowHeight = shadowSize; // 縦幅

		// 深度情報を書き込む
		m_cascadeShadowMapRTPack[i].CreateRenderTarget(shadowWidth, shadowHeight, true, DXGI_FORMAT_R32_FLOAT);
		m_cascadeShadowMapRTPack[i].ClearTexture(kRedColor);
	}

	SetDissolveTexture(*KdAssets::Instance().m_textures.GetData("Asset/Textures/System/WhiteNoise.png"));

	return true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// シェーダー本体の解放
// 利用していたコンスタントバッファの解放
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::Release()
{
	KdSafeRelease(m_VS_Lit);
	KdSafeRelease(m_VS_GenDepthFromLight);
	KdSafeRelease(m_VS_UnLit);

	KdSafeRelease(m_inputLayout);
	
	KdSafeRelease(m_PS_Lit);
	KdSafeRelease(m_PS_GenDepthFromLight);
	KdSafeRelease(m_PS_UnLit);
	KdSafeRelease(m_PS_Toon);
	KdSafeRelease(m_PS_Gradation);
	KdSafeRelease(m_PS_GrayScale);
	KdSafeRelease(m_PS_Effect);
	KdSafeRelease(m_PS_RimLight);

	m_cb0_Obj.Release();
	m_cb1_Mesh.Release();
	m_cb2_Material.Release();

	// スキンメッシュ対応
	m_cb3_Bone.Release();

	// エフェクト用
	m_cb4_Effect.Release();
}

void KdStandardShader::GetProjectctionDecompose(float& _OutFov, float& _outAspect, float& _outNear, float& _outFar, Math::Matrix& _outProjMat)
{
	Math::Matrix mProj = KdShaderManager::Instance().GetCameraCB().mProj;

	// 行列から各種パラメータを逆算
	_OutFov = DirectX::XMConvertToDegrees(2.0f * atan(1.0f / mProj._22));	// 垂直画角
	_outAspect = mProj._22 / mProj._11;									// アスペクト比
	_outNear = mProj._43 / (mProj._33 - 1.0f);							// ニアクリップ距離
	_outFar = mProj._43 / (mProj._33 + 1.0f);								// ファークリップ距離

	// ニアとファーを設定
	_outNear = Cascade::MinClip;
	_outFar = Cascade::MaxClip;

	// 投影行列の再構築
	_outProjMat = DirectX::XMMatrixPerspectiveFovLH
	(
		DirectX::XMConvertToRadians(_OutFov),
		_outAspect,
		_outNear,
		_outFar
	);

}

void KdStandardShader::CascadeShadowMapChangea(const DirectX::BoundingBox& _BBox)
{
	// 1. カメラ情報から FOV/アスペクト/near/far を取得
	float fov, aspect, nearClip, farClip;
	Math::Matrix mProj;
	GetProjectctionDecompose(fov, aspect, nearClip, farClip, mProj);

	// カメラフラスタム作成（CSM 用に near/far を制限済みの mProj を使う）
	DirectX::BoundingFrustum camFrustum;
	DirectX::BoundingFrustum::CreateFromMatrix(camFrustum, mProj);

	// カメラ位置と向き設定
	Math::Vector3 CamSize;
	Math::Quaternion CamDir;
	Math::Vector3 CameraPos;
	KdShaderManager::Instance().GetCameraCB().mView.Invert().Decompose(CamSize, CamDir, CameraPos);
	camFrustum.Origin = CameraPos;
	camFrustum.Orientation = CamDir;

	// 分割位置計算（near/far は GetProjectctionDecompose で既に制限済み）
	float useNear = std::max(nearClip, Cascade::MinClip);
	float useFar = std::min(farClip, Cascade::MaxClip);

	float cascadeSplits[Cascade::Num + 1];
	for (int i = 0; i <= Cascade::Num; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(Cascade::Num);
		float log = useNear * std::pow(useFar / useNear, t);
		float uniform = useNear + (useFar - useNear) * t;
		cascadeSplits[i] = std::lerp(uniform, log, Cascade::SplitLambda);
	}

	// 初期化
	m_cascadeCount = 0;

	// 光方向・上方向
	Math::Vector3 lightDir = KdShaderManager::Instance().GetLightCB().DirLight_Dir;
	Math::Vector3 upVec = (lightDir == Math::Vector3::Up) ? Math::Vector3::Right : Math::Vector3::Up;

	// 各カスケードごとにフラスタム作って判定
	for (int cascadeIdx = 0; cascadeIdx < Cascade::Num; ++cascadeIdx)
	{
		float cNear = cascadeSplits[cascadeIdx];
		float cFar = cascadeSplits[cascadeIdx + 1] * 1.1f; // 少し余裕

		// カスケードフラスタム作成
		DirectX::BoundingFrustum cascadeFrustum;
		DirectX::BoundingFrustum::CreateFromMatrix(cascadeFrustum,
			DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), aspect, cNear, cFar));
		cascadeFrustum.Origin = CameraPos;
		cascadeFrustum.Orientation = CamDir;

		// ワールド空間でフラスタムの 8 コーナー取得
		DirectX::XMFLOAT3 frustCornersWS[8];
		cascadeFrustum.GetCorners(frustCornersWS);

		// フラスタムを囲むワールドAABB
		DirectX::BoundingBox frustAABB_World;
		DirectX::BoundingBox::CreateFromPoints(frustAABB_World, 8, frustCornersWS, sizeof(DirectX::XMFLOAT3));

		// ライトビュー行列（フラスタム中心をターゲット）
		Math::Vector3 center = frustAABB_World.Center;
		Math::Vector3 lightPos = center - lightDir * frustAABB_World.Extents.z * 2.0f;
		Math::Matrix lightView = DirectX::XMMatrixLookAtLH(lightPos, center, upVec);

		// フラスタムのコーナーをライト空間へ変換してライト空間AABBを作成
		DirectX::XMFLOAT3 frustCornersLS[8];
		for (int c = 0; c < 8; ++c)
		{
			DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&frustCornersWS[c]);
			DirectX::XMVECTOR t = DirectX::XMVector3TransformCoord(v, lightView);
			DirectX::XMStoreFloat3(&frustCornersLS[c], t);
		}
		DirectX::BoundingBox frustAABB_LS;
		DirectX::BoundingBox::CreateFromPoints(frustAABB_LS, 8, frustCornersLS, sizeof(DirectX::XMFLOAT3));

		// 重要: ライト方向（LS の Z）に余白を追加して、視錐台外キャスターを許容
		const float ShadowCasterMargin = 30.0f; // シーンに合わせて調整（最大影長/最大オブジェクトサイズなど）
		frustAABB_LS.Extents.z += ShadowCasterMargin;

		// メッシュの AABB をワールド→ライト空間に変換して作成
		DirectX::XMFLOAT3 meshCornersWS[8];
		_BBox.GetCorners(meshCornersWS);
		DirectX::XMFLOAT3 meshCornersLS[8];
		for (int c = 0; c < 8; ++c)
		{
			DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&meshCornersWS[c]);
			DirectX::XMVECTOR t = DirectX::XMVector3TransformCoord(v, lightView);
			DirectX::XMStoreFloat3(&meshCornersLS[c], t);
		}
		DirectX::BoundingBox meshAABB_LS;
		DirectX::BoundingBox::CreateFromPoints(meshAABB_LS, 8, meshCornersLS, sizeof(DirectX::XMFLOAT3));

		const float XYMarginScale = 10.05f;
		frustAABB_LS.Extents.x *= XYMarginScale;
		frustAABB_LS.Extents.y *= XYMarginScale;

		// 判定：ライト空間の AABB 同士が重なれば該当カスケードに描画が必要
		if (frustAABB_LS.Intersects(meshAABB_LS))
		{
			m_cascadeCount |= (1u << cascadeIdx); // シフト量に使う
		}
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画用マテリアル情報の転送
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// それぞれのマテリアルの影響倍率値とテクスチャを設定
// BaseColor：基本色 / Emissive：自己発光色 / Metalic：金属性(テカテカ) / Roughness：粗さ(材質の色の反映度)
// テクスチャは法線マップ以外は未設定なら白1ピクセルのシステムテクスチャを指定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::WriteMaterial(const KdMaterial& material, const Math::Vector4& colRate, const Math::Vector3& emiRate)
{
	//-----------------------
	// マテリアル情報を定数バッファへ書き込む
	//-----------------------
	m_cb2_Material.Work().BaseColor = material.m_baseColorRate * colRate;
	m_cb2_Material.Work().Emissive = material.m_emissiveRate * emiRate;
	m_cb2_Material.Work().Metallic = material.m_metallicRate;
	m_cb2_Material.Work().Roughness = material.m_roughnessRate;
	m_cb2_Material.Write();

	//-----------------------
	// テクスチャセット
	//-----------------------
	ID3D11ShaderResourceView* srvs[4];

	srvs[0] = material.m_baseColorTex ? material.m_baseColorTex->WorkSRView() : KdDirect3D::Instance().GetWhiteTex()->WorkSRView();
	srvs[1] = material.m_metallicRoughnessTex ? material.m_metallicRoughnessTex->WorkSRView() : KdDirect3D::Instance().GetWhiteTex()->WorkSRView();
	srvs[2] = material.m_emissiveTex ? material.m_emissiveTex->WorkSRView() : KdDirect3D::Instance().GetWhiteTex()->WorkSRView();
	srvs[3] = material.m_normalTex ? material.m_normalTex->WorkSRView() : KdDirect3D::Instance().GetNormalTex()->WorkSRView();

	// セット
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(0, _countof(srvs), srvs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ポリゴンがどの方向に向いていても光の影響を正面からに受けるように頂点の法線を変換
// 2Dキャラクタを描画する時などは必要
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::ConvertNormalsFor2D(std::vector<KdPolygon::Vertex>& target, const Math::Matrix& mWorld)
{
	// 平行光の向き
	const Math::Vector3& dirLight_Dir = KdShaderManager::Instance().GetLightCB().DirLight_Dir;

	// どの角度を向いていても表面は常に光の方向を向いている状態：横向きの板ポリが暗くならない対策
	Math::Vector3 normal = Math::Vector3::TransformNormal(-dirLight_Dir, mWorld.Invert());
	Math::Vector3 tangent = (normal != Math::Vector3::Up) ?
		normal.Cross(Math::Vector3::Up) : normal.Cross(Math::Vector3::Right);

	for (size_t i = 0; i < target.size(); ++i)
	{
		target[i].normal = normal;
		target[i].tangent = tangent;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// オブジェクト定数バッファを初期状態に戻す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::ResetCBObject()
{
	m_cb0_Obj.Work() = cbObject();

	m_cb0_Obj.Write();

	m_dirtyCBObj = false;
}
