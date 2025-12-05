#include "Framework/KdFramework.h"
#include "KdShaderManager.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 全てのシェーダーオブジェクトの生成
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// 描画するシェーダー本体の作成（メモリ確保）
// 全ての描画に必要であろうカメラ・ライトの定数バッファの作成
// 描画に関する様々なステートの作成（実行中に切り替える用：切り替えるたびに新しく作るのは無駄
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::Init()
{
	//============================================
	// シェーダ
	//============================================
	m_spriteShader.Init();
	m_StandardShader.Init();
	m_postProcessShader.Init();

	//============================================
	// 定数バッファ
	//============================================

	// カメラ：ビュー行列・射影行列
	m_cb7_Camera.Create();
	KdDirect3D::Instance().WorkDevContext()->VSSetConstantBuffers(7, 1, m_cb7_Camera.GetAddress());
	KdDirect3D::Instance().WorkDevContext()->PSSetConstantBuffers(7, 1, m_cb7_Camera.GetAddress());

	// フォグ：高さフォグ・距離フォグ
	m_cb8_Fog.Create();
	m_cb8_Fog.Write();
	KdDirect3D::Instance().WorkDevContext()->VSSetConstantBuffers(8, 1, m_cb8_Fog.GetAddress());
	KdDirect3D::Instance().WorkDevContext()->PSSetConstantBuffers(8, 1, m_cb8_Fog.GetAddress());

	// ライト：平行光源・点光源・環境光
	m_cb9_Light.Create();
	m_cb9_Light.Write();
	KdDirect3D::Instance().WorkDevContext()->VSSetConstantBuffers(9, 1, m_cb9_Light.GetAddress());
	KdDirect3D::Instance().WorkDevContext()->PSSetConstantBuffers(9, 1, m_cb9_Light.GetAddress());

	//============================================
	// パイプラインステート関係
	//============================================
	//深度ステンシルステート作成（奥行情報の使い方・手前にあるものを無視して描画したりできる
	m_depthStencilStates[(int)KdDepthStencilState::ZEnable]			= KdDirect3D::Instance().CreateDepthStencilState(true, true);
	m_depthStencilStates[(int)KdDepthStencilState::ZWriteDisable]	= KdDirect3D::Instance().CreateDepthStencilState(true, false);
	m_depthStencilStates[(int)KdDepthStencilState::ZDisable]		= KdDirect3D::Instance().CreateDepthStencilState(false, false);

	// 初期深度ステートの設定
	KdDirect3D::Instance().WorkDevContext()->OMSetDepthStencilState(m_depthStencilStates[(int)KdDepthStencilState::ZEnable], 0);

	// ラスタライザステート作成（ポリゴンの任意の面の描画を省略できる、処理を軽減する目的で運用する事が多い
	m_rasterizerStates[(int)KdRasterizerState::CullNone]	= KdDirect3D::Instance().CreateRasterizerState(D3D11_CULL_NONE, D3D11_FILL_SOLID, true, false);
	m_rasterizerStates[(int)KdRasterizerState::CullFront]	= KdDirect3D::Instance().CreateRasterizerState(D3D11_CULL_FRONT, D3D11_FILL_SOLID, true, false);
	m_rasterizerStates[(int)KdRasterizerState::CullBack]	= KdDirect3D::Instance().CreateRasterizerState(D3D11_CULL_BACK, D3D11_FILL_SOLID, true, false);
	m_rasterizerStates[(int)KdRasterizerState::CullFrontWireframe] = KdDirect3D::Instance().CreateRasterizerState(D3D11_CULL_FRONT, D3D11_FILL_WIREFRAME, true, false);

	// 初期ラスタライザステートの設定
	KdDirect3D::Instance().WorkDevContext()->RSSetState(m_rasterizerStates[(int)KdRasterizerState::CullBack]);

	// ブレンドステート作成（ピクセルの最終色を決めるときに既に塗られている色と、どう合成するのかの選択ができる
	m_blendStates[(int)KdBlendState::Alpha]	= KdDirect3D::Instance().CreateBlendState(KdBlendMode::Alpha);
	m_blendStates[(int)KdBlendState::Add]	= KdDirect3D::Instance().CreateBlendState(KdBlendMode::Add);

	// 初期ブレンドステートの設定
	KdDirect3D::Instance().WorkDevContext()->OMSetBlendState(m_blendStates[(int)KdBlendState::Alpha], Math::Color(0, 0, 0, 0), 0xFFFFFFFF);

	// サンプラーステート作成（テクスチャのピクセル色を取得する際にどのような補間や扱いで取得するかを選択できる
	m_samplerStates[(int)KdSamplerState::Anisotropic_Wrap]	= KdDirect3D::Instance().CreateSamplerState(KdSamplerFilterMode::Anisotropic, 4, KdSamplerAddressingMode::Wrap, false);
	m_samplerStates[(int)KdSamplerState::Anisotropic_Clamp] = KdDirect3D::Instance().CreateSamplerState(KdSamplerFilterMode::Anisotropic, 4, KdSamplerAddressingMode::Clamp, false);
	m_samplerStates[(int)KdSamplerState::Linear_Clamp]		= KdDirect3D::Instance().CreateSamplerState(KdSamplerFilterMode::Linear, 0, KdSamplerAddressingMode::Clamp, false);
	m_samplerStates[(int)KdSamplerState::Linear_Clamp_Cmp]	= KdDirect3D::Instance().CreateSamplerState(KdSamplerFilterMode::Linear, 0, KdSamplerAddressingMode::Clamp, true);
	m_samplerStates[(int)KdSamplerState::Point_Wrap]		= KdDirect3D::Instance().CreateSamplerState(KdSamplerFilterMode::Point, 0, KdSamplerAddressingMode::Wrap, false);
	m_samplerStates[(int)KdSamplerState::Point_Clamp]		= KdDirect3D::Instance().CreateSamplerState(KdSamplerFilterMode::Point, 0, KdSamplerAddressingMode::Clamp, false);
	
	// 初期サンプラーステートの設定
	if (m_pixelArtStyle)
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetSamplers(0, 1, &m_samplerStates[(int)KdSamplerState::Point_Wrap]);
	}
	else
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetSamplers(0, 1, &m_samplerStates[(int)KdSamplerState::Anisotropic_Wrap]);
	}

	m_ambientController.Init();
}


//==========================
//
// 描画パイプライン系の設定
//
//==========================

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 頂点シェーダのセット（現行のシェーダーと同じ場合はキャンセル）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdShaderManager::SetVertexShader(ID3D11VertexShader* pSetVS)
{
	if (!pSetVS) { return false; }

	ID3D11VertexShader* pNowVS = nullptr;
	KdDirect3D::Instance().WorkDevContext()->VSGetShader(&pNowVS, nullptr, nullptr);

	// セットしようとしているシェーダーが現行と同じならキャンセル
	bool needChange = pNowVS != pSetVS;

	if (needChange)
	{
		KdDirect3D::Instance().WorkDevContext()->VSSetShader(pSetVS, nullptr, 0);
	}

	KdSafeRelease(pNowVS);

	return needChange;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ピクセルシェーダのセット（現行のシェーダーと同じ場合はキャンセル）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdShaderManager::SetPixelShader(ID3D11PixelShader* pSetPS)
{
	if (!pSetPS) { return false; }

	ID3D11PixelShader* pNowPS = nullptr;
	KdDirect3D::Instance().WorkDevContext()->PSGetShader(&pNowPS, nullptr, nullptr);

	// セットしようとしているシェーダーが現行と同じならキャンセル
	bool needChange = pNowPS != pSetPS;

	if (needChange)
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetShader(pSetPS, nullptr, 0);
	}

	KdSafeRelease(pNowPS);

	return needChange;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 頂点入力レイアウトのセット（現行のレイアウトと同じ場合はキャンセル）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdShaderManager::SetInputLayout(ID3D11InputLayout* pSetLayout)
{
	if (!pSetLayout) { return false; }

	ID3D11InputLayout* pNowLayout = nullptr;

	KdDirect3D::Instance().WorkDevContext()->IAGetInputLayout(&pNowLayout);

	// セットしようとしているレイアウトが現行と同じならキャンセル
	bool needChange = pNowLayout != pSetLayout;

	if (needChange)
	{
		KdDirect3D::Instance().WorkDevContext()->IASetInputLayout(pSetLayout);
	}

	KdSafeRelease(pNowLayout);

	return needChange;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 頂点シェーダーの定数バッファのセット（現行の定数バッファと同じ場合はキャンセル）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdShaderManager::SetVSConstantBuffer(int startSlot, ID3D11Buffer* const* pSetVSBuffer)
{
	if (!pSetVSBuffer) { return false; }

	ID3D11Buffer* pNowVSBuffer = nullptr;

	KdDirect3D::Instance().WorkDevContext()->VSGetConstantBuffers(startSlot, 1, &pNowVSBuffer);

	// セットしようとしている定数バッファが現行と同じならキャンセル
	bool needChange = pNowVSBuffer != *pSetVSBuffer;

	if (needChange)
	{
		KdDirect3D::Instance().WorkDevContext()->VSSetConstantBuffers(startSlot, 1, pSetVSBuffer);
	}

	KdSafeRelease(pNowVSBuffer);

	return needChange;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ピクセルシェーダーの定数バッファのセット（現行の定数バッファと同じ場合はキャンセル）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdShaderManager::SetPSConstantBuffer(int startSlot, ID3D11Buffer* const* pSetPSBuffer)
{
	if (!pSetPSBuffer) { return false; }

	ID3D11Buffer* pNowPSBuffer = nullptr;

	KdDirect3D::Instance().WorkDevContext()->PSGetConstantBuffers(startSlot, 1, &pNowPSBuffer);

	bool needChange = pNowPSBuffer != *pSetPSBuffer;

	if (needChange)
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetConstantBuffers(startSlot, 1, pSetPSBuffer);
	}

	KdSafeRelease(pNowPSBuffer);

	return needChange;
}


//==========================
//
// パイプラインステートの変更
//
//==========================

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 深度ステンシルステートの変更（現行と同じステートの場合はキャンセル
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::ChangeDepthStencilState(KdDepthStencilState stateId)
{
	ID3D11DepthStencilState* pNowDs = nullptr;
	KdDirect3D::Instance().WorkDevContext()->OMGetDepthStencilState(&pNowDs, 0);

	if (pNowDs != m_depthStencilStates[(int)stateId])
	{
		KdDirect3D::Instance().WorkDevContext()->OMSetDepthStencilState(m_depthStencilStates[(int)stateId], 0);
	}

	m_ds_Undo.push(pNowDs);

	KdSafeRelease(pNowDs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 深度ステンシルステートを変更直前のステートに戻す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::UndoDepthStencilState()
{
	if (!m_ds_Undo.size()) { return; }

	ID3D11DepthStencilState* pNowDs = nullptr;
	KdDirect3D::Instance().WorkDevContext()->OMGetDepthStencilState(&pNowDs, 0);

	if (pNowDs != m_ds_Undo.top())
	{
		KdDirect3D::Instance().WorkDevContext()->OMSetDepthStencilState(m_ds_Undo.top(), 0);
	}

	m_ds_Undo.pop();

	KdSafeRelease(pNowDs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ラスタライザステートの変更（現行と同じステートの場合はキャンセル
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::ChangeRasterizerState(KdRasterizerState stateId)
{
	ID3D11RasterizerState* pNowRs = nullptr;
	KdDirect3D::Instance().WorkDevContext()->RSGetState(&pNowRs);

	if (pNowRs != m_rasterizerStates[(int)stateId])
	{
		KdDirect3D::Instance().WorkDevContext()->RSSetState(m_rasterizerStates[(int)stateId]);
	}

	m_rs_Undo.push(pNowRs);

	KdSafeRelease(pNowRs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ラスタライザステンシルステートを変更直前のステートに戻す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::UndoRasterizerState()
{
	if (!m_rs_Undo.size()) { return; }

	ID3D11RasterizerState* pNowRs = nullptr;
	KdDirect3D::Instance().WorkDevContext()->RSGetState(&pNowRs);

	if (pNowRs != m_rs_Undo.top())
	{
		KdDirect3D::Instance().WorkDevContext()->RSSetState(m_rs_Undo.top());
	}

	m_rs_Undo.pop();

	KdSafeRelease(pNowRs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ブレンドステートの変更（現行と同じステートの場合はキャンセル
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::ChangeBlendState(KdBlendState stateId)
{
	ID3D11BlendState* pNowBs = nullptr;
	KdDirect3D::Instance().WorkDevContext()->OMGetBlendState(&pNowBs, nullptr, nullptr);

	if (pNowBs != m_blendStates[(int)stateId])
	{
		KdDirect3D::Instance().WorkDevContext()->OMSetBlendState(m_blendStates[(int)stateId], Math::Color(0, 0, 0, 0), 0xFFFFFFFF);
	}

	m_bs_Undo.push(pNowBs);

	KdSafeRelease(pNowBs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ブレンドステートを変更直前のステートに戻す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::UndoBlendState()
{
	if (!m_bs_Undo.size()) { return; }

	ID3D11BlendState* pNowBs = nullptr;
	KdDirect3D::Instance().WorkDevContext()->OMGetBlendState(&pNowBs, nullptr, nullptr);
	
	if (pNowBs != m_bs_Undo.top())
	{
		KdDirect3D::Instance().WorkDevContext()->OMSetBlendState(m_bs_Undo.top(), Math::Color(0, 0, 0, 0), 0xFFFFFFFF);
	}

	m_bs_Undo.pop();

	KdSafeRelease(pNowBs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// サンプラーステートの変更（現行と同じステートの場合はキャンセル
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::ChangeSamplerState(KdSamplerState stateId, int slot)
{
	ID3D11SamplerState* pNowSs = nullptr;
	KdDirect3D::Instance().WorkDevContext()->PSGetSamplers(slot, 1, &pNowSs);

	if (pNowSs != m_samplerStates[(int)stateId])
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetSamplers(slot, 1, &m_samplerStates[(int)stateId]);
	}

	m_ss_Undo.push(pNowSs);

	KdSafeRelease(pNowSs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// サンプラーステートを変更直前のステートに戻す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::UndoSamplerState(int slot)
{
	if (!m_ss_Undo.size()) { return; }

	ID3D11SamplerState* pNowSs = nullptr;
	KdDirect3D::Instance().WorkDevContext()->PSGetSamplers(0, 1, &pNowSs);

	if (pNowSs != m_ss_Undo.top())
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetSamplers(slot, 1, &m_ss_Undo.top());
	}

	m_ss_Undo.pop();

	KdSafeRelease(pNowSs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// カメラ情報をGPUに転送
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::WriteCBCamera(const Math::Matrix& cam, const Math::Matrix& proj)
{
	cbCamera& camera = m_cb7_Camera.Work();

	camera.mView = cam.Invert();

	camera.mProj = proj;
	camera.mProjInv = proj.Invert();

	camera.CamPos = cam.Translation();

	m_cb7_Camera.Write();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// フォグの有効フラグをGPUに転送
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::WriteCBFogEnable(bool distance, bool height)
{
	cbFog& fog = m_cb8_Fog.Work();

	fog.DistanceFogEnable = distance;
	fog.HeightFogEnable = height;

	m_cb8_Fog.Write();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 距離フォグのデータをGPUに転送
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::WriteCBDistanceFog(const Math::Vector3& col, float density)
{
	cbFog& fog = m_cb8_Fog.Work();

	fog.DistanceFogColor = col;
	fog.DistanceFogDensity = density;

	m_cb8_Fog.Write();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 高さフォグのデータをGPUに転送
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::WriteCBHeightFog(const Math::Vector3& col, float top, float bottom, float beginDistance)
{
	cbFog& fog = m_cb8_Fog.Work();

	fog.HeightFogColor = col;
	fog.HeightFogTopValue = top;
	fog.HeightFogBottomValue = bottom;
	fog.HeightFogBeginDistance = beginDistance;

	m_cb8_Fog.Write();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 環境光のデータをGPUに転送
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::WriteCBAmbientLight(const Math::Vector4& col)
{
	m_cb9_Light.Work().AmbientLight = col;

	m_cb9_Light.Write();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 平行光のデータをGPUに転送
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::WriteCBDirectionalLight(const Math::Vector3& dir, const Math::Vector3& col)
{
	cbLight& light = m_cb9_Light.Work();

	light.DirLight_Dir = dir;
	light.DirLight_Color = col;

	m_cb9_Light.Write();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 影生成エリアのデータをGPUに転送
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::WriteCBShadowArea(const Math::Matrix& proj, float dirLightHeight, const Math::Vector3& center)
{
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 点光源の配列データをGPUに転送
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdShaderManager::WriteCBPointLight(const std::list<PointLight>& pointLights)
{
	cbLight& light = m_cb9_Light.Work();

	light.PointLight_Num = (signed)pointLights.size();

	UINT pointIndex = 0;

	for (const PointLight& pointlight : pointLights)
	{
		light.PointLights[pointIndex] = pointlight;

		++pointIndex;
	}

	m_cb9_Light.Write();
}

void KdShaderManager::WriteCBShadowAreaCSM()
{
	Math::Vector3 _LightDir = m_cb9_Light.Get().DirLight_Dir;
	_LightDir.Normalize();
	Math::Vector3 _LightPos = m_cb7_Camera.Get().CamPos;
	Math::Vector3 _UpVec = (_LightDir == Math::Vector3::Up) ? Math::Vector3::Right : Math::Vector3::Up;

	// 1. カメラ情報からFOV・アスペクト比・ニアファー・ファーを取得
	float _FOV;
	float _Aspect;
	float _Near;
	float _Far;

	Math::Matrix _ProjMat;

	m_StandardShader.GetProjectctionDecompose(_FOV, _Aspect, _Near, _Far, _ProjMat);

	// CSMは常に指定レンジを使用（カメラが 0.01～2000 でも影は 1～300 に限定）
	_Near = Cascade::MinClip;   // 1.0f
	_Far = Cascade::MaxClip;   // 300.0f

	// 以降のフラスタム作成にこのレンジを使用
	DirectX::BoundingFrustum _FullFrustum;
	_ProjMat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(_FOV), _Aspect, _Near, _Far);
	DirectX::BoundingFrustum::CreateFromMatrix(_FullFrustum, _ProjMat);

	// フラスタムの位置角度を設定するためにカメラのビュー行列をインバートして行列情報を取り込む
	Math::Vector3		_CameraSize;
	Math::Quaternion	_CameraDir;
	Math::Vector3		_CameraPos;

	m_cb7_Camera.Get().mView.Invert().Decompose(_CameraSize, _CameraDir, _CameraPos);

	// フラスタムの位置角度を設定
	_FullFrustum.Origin = _CameraPos;
	_FullFrustum.Orientation = _CameraDir;

	// 作成したフラスタムからニアとファーを取得
	_Near = _FullFrustum.Near;
	_Far = _FullFrustum.Far;

	// カスケードするフラスタムのニアファーを設定
	float _CascadeSplits[Cascade::Num + 1]; // 3カスケードなら4要素

	// 分割するカスケードごとのニアファーを計算
	for (int i = 0; i <= Cascade::Num; i++)
	{
		float t = (float)i / Cascade::Num; // 0, 1/3, 2/3, 1
		float linear = _Near + (_Far - _Near) * t;
		float log = _Near * powf(_Far / _Near, t);

		_CascadeSplits[i] = std::lerp(linear, log, Cascade::SplitLambda);
	}

	// フラスタムをカスケード用に複製する
	DirectX::BoundingFrustum _CascadeFrustum;

	float _CascadeNear[Cascade::Num], _CascadeFar[Cascade::Num];

	// カスケードごとにDirLight_mVPを設定
	for (int i = 0; i < Cascade::Num; i++)
	{
		// 3. カスケード用のフラスタムを作成
		{
			_Near = _CascadeSplits[i];
			_Far = _CascadeSplits[i + 1] * 1.1f;

			_ProjMat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(_FOV), _Aspect, _Near, _Far);

			DirectX::BoundingFrustum::CreateFromMatrix(_CascadeFrustum, _ProjMat);
		}

		// フラスタムの位置角度を設定
		_CascadeFrustum.Origin = _CameraPos;
		_CascadeFrustum.Orientation = _CameraDir;

		_CascadeNear[i] = _Near;
		_CascadeFar[i] = _Far;

		// 計算したフルフラスタムからAABBを作成しライトビュー行列の作成
		Math::Matrix _LightView;
		Math::Vector3 _Center; // フラスタムの中心点
		{
			// フラスタムから角を取得
			DirectX::XMFLOAT3 _FrustumCorners[8];
			_CascadeFrustum.GetCorners(_FrustumCorners);


			DirectX::BoundingBox _FrustumBox;
			_FrustumBox.CreateFromPoints(_FrustumBox, 8, (Math::Vector3*)_FrustumCorners, sizeof(Math::Vector3));

			_Center = _FrustumBox.Center;

			_LightPos = _Center - _LightDir * _FrustumBox.Extents.z * 2.0f;

			_LightView = DirectX::XMMatrixLookAtLH(_LightPos, _Center, _UpVec);
		}

		// 計算したカスケードフラスタムからAABBを作成
		DirectX::BoundingBox _CascadeBox;

		_CascadeBox.Center = _Center;
		_CascadeBox.Extents = { (_CascadeFrustum.Far - _CascadeFrustum.Near),(_CascadeFrustum.Far - _CascadeFrustum.Near) ,(_CascadeFrustum.Far - _CascadeFrustum.Near) };

		DirectX::XMFLOAT3 _FrustumCorners[8];
		_CascadeBox.GetCorners(_FrustumCorners);

		Math::Vector3 _FrustumCornersLS[8];
		for (int c = 0; c < 8; ++c) {
			Math::Vector3 cornerWS = _FrustumCorners[c];
			Math::Vector3 cornerLS = DirectX::XMVector3TransformCoord(cornerWS, _LightView);
			_FrustumCornersLS[c] = cornerLS;
		}

		// 4. ライト空間で AABB を作成
		DirectX::BoundingBox _CascadeBoxLS;
		DirectX::BoundingBox::CreateFromPoints(_CascadeBoxLS, 8, _FrustumCornersLS, sizeof(Math::Vector3));

		// 5. テクセルスナップでシャドウマップの大きさに準じて中心位置を調整
		{
			// それぞれのシャドウマップのサイズでテクセルスナップ適用
			int _ShadowMapExp = Cascade::MaxExp - i;				// シフト演算でずらす数(9～11)
			float _ShadowSize = 1 << _ShadowMapExp; // 近：2048*2048　中：1024*1024　遠：512*512

			// シャドウマップのX軸Y軸の大きさを求める　（正方形なので同じサイズを代入）
			float _ShadowMapResX = _ShadowSize; // シャドウマップ解像度X
			float _ShadowMapResY = _ShadowSize; // シャドウマップ解像度Y

			// それぞれのシャドウマップに合わせたテクセルサイズにする
			float _TexelSizeX = (_CascadeBoxLS.Extents.x * 2.0f) / _ShadowMapResX;
			float _TexelSizeY = (_CascadeBoxLS.Extents.y * 2.0f) / _ShadowMapResY;

			// テクセルで直した中心位置にする
			_CascadeBoxLS.Center.x = std::round(_CascadeBoxLS.Center.x / _TexelSizeX) * _TexelSizeX;
			_CascadeBoxLS.Center.y = std::round(_CascadeBoxLS.Center.y / _TexelSizeY) * _TexelSizeY;

			//　Z 方向もスナップ
			const float _TexelSizeZ = (_CascadeBoxLS.Extents.z * 2.0f) / _ShadowMapResY;
			_CascadeBoxLS.Center.z = std::round(_CascadeBoxLS.Center.z / _TexelSizeZ) * _TexelSizeZ;
		}

		// 6. この AABB をもとに正射影行列を作る
		Math::Matrix _LightProj = DirectX::XMMatrixOrthographicOffCenterLH(
			_CascadeBoxLS.Center.x - _CascadeBoxLS.Extents.x, _CascadeBoxLS.Center.x + _CascadeBoxLS.Extents.x,
			_CascadeBoxLS.Center.y - _CascadeBoxLS.Extents.y, _CascadeBoxLS.Center.y + _CascadeBoxLS.Extents.y,
			_CascadeBoxLS.Center.z - _CascadeBoxLS.Extents.z, _CascadeBoxLS.Center.z + _CascadeBoxLS.Extents.z
		);

		Math::Matrix _SynthesisMat = _LightView * _LightProj;

		m_cb9_Light.Work().DirLight_mVP[i] = _SynthesisMat;

		m_cb9_Light.Write();
	}

	m_cb7_Camera.Work().CascadeFar = { _CascadeFar[0],_CascadeFar[1] ,_CascadeFar[2] };
	m_cb7_Camera.Work().CascadeNear = { _CascadeNear[0],_CascadeNear[1], _CascadeNear[2] };
	m_cb7_Camera.Write();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// パラメータの解放：シェーダー本体・共通の定数バッファ・各パイプラインステート
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
#pragma warning(disable:4239)
void KdShaderManager::Release()
{
	m_StandardShader.Release();
	m_postProcessShader.Release();
	m_spriteShader.Release();

	m_cb7_Camera.Release();
	m_cb8_Fog.Release();
	m_cb9_Light.Release();

	//深度ステンシルステート開放
	for (auto& state : m_depthStencilStates)
	{
		KdSafeRelease(state);
	}

	auto ds = std::stack<ID3D11DepthStencilState*>();
	m_ds_Undo.swap(ds);

	// ラスタライザステート解放
	for (auto& state : m_rasterizerStates)
	{
		KdSafeRelease(state);
	}

	auto rs = std::stack<ID3D11RasterizerState*>();
	m_rs_Undo.swap(rs);

	// ブレンドステート解放
	for (auto& state : m_blendStates)
	{
		KdSafeRelease(state);
	}

	auto bs = std::stack<ID3D11BlendState*>();
	m_bs_Undo.swap(bs);

	// サンプラーステート解放
	for (auto& state : m_samplerStates)
	{
		KdSafeRelease(state);
	}

	auto ss = std::stack<ID3D11SamplerState*>();
	m_ss_Undo.swap(ss);
}
#pragma warning(default:4239)
