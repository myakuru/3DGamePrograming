#include "KdAmbientController.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// シェーダーマネージャで設定したシェーダーの初期値を取得してくる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::Init()
{
	// 光源のパラメータ
	{
		const KdShaderManager::cbLight& cbLight = KdShaderManager::Instance().GetLightCB();

		m_parameter.m_ambientLightColor = cbLight.AmbientLight;

		m_parameter.m_directionalLightDir = cbLight.DirLight_Dir;
		m_parameter.m_directionalLightDir.Normalize();

		m_parameter.m_directionalLightColor = cbLight.DirLight_Color;
	}

	// フォグのパラメータ
	{
		const KdShaderManager::cbFog& cbFog = KdShaderManager::Instance().GetFogCB();

		m_parameter.m_distanceFogColor = cbFog.DistanceFogColor;
		m_parameter.m_distanceFogDensity = cbFog.DistanceFogDensity;
		
		m_parameter.m_heightFogColor = cbFog.HeightFogColor;
		m_parameter.m_heightFogTopValue= cbFog.HeightFogTopValue;
		m_parameter.m_heightFogBottomValue = cbFog.HeightFogBottomValue;
		m_parameter.m_heightFogBeginDistance = cbFog.HeightFogBeginDistance;
	}

	// 影用深度バッファ生成情報の初期化
	SetDirLightShadowArea({ 100.0f, 100.0f }, 30.0f);

	// 光のパラメータの反映
	// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
	WriteLightParams();

	// フォグのパラメータの反映
	// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
	WriteFogParams();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 点光源は毎フレーム登録する形なので0クリアする
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::Update()
{
	m_pointLights.clear();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画の直前の更新が終わったタイミングで実施
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::Draw()
{
	// 光のパラメータの反映
	// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
	WriteLightParams();

	// フォグのパラメータの反映
	// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
	WriteFogParams();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ポイントライトの追加
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::AddPointLight(const Math::Vector3& Color, float Radius, const Math::Vector3& Pos, bool IsBright)
{
	m_pointLights.push_back(PointLight(Color, Radius, Pos, IsBright));
}

void KdAmbientController::AddPointLight(const PointLight& pointLight)
{
	m_pointLights.push_back(pointLight);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 平行光影の生成エリアの設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::SetDirLightShadowArea(const Math::Vector2& lightingArea, float dirLightHeight)
{
	// lightingArea.x 幅, lightingArea.y 奥行（正射影：左右 / 上下）
		// プレイヤー中心にしたいのでここは単にサイズだけ保持
	float width = lightingArea.x;
	float height = lightingArea.y;

	m_shadowProj = DirectX::XMMatrixOrthographicLH(width, height, 0.0f, dirLightHeight * 2.0f);
	m_dirLightHeight = dirLightHeight;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 平行光の設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::SetDirLight(const Math::Vector3& dir, const Math::Vector3& col)
{
	m_parameter.m_directionalLightDir = dir;
	m_parameter.m_directionalLightDir.Normalize();

	m_parameter.m_directionalLightColor = col;

	m_dirtyLightDir = true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 環境光の設定
// アルファ値の変更でデフォルトの暗さを指定できる。0にした場合は点光源の周囲以外は真っ暗
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::SetAmbientLight(const Math::Vector4& col)
{
	m_parameter.m_ambientLightColor = col;

	m_dirtyLightAmb = true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// フォグの有効/無効をそれぞれ切り替える
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::SetFogEnable(bool distance, bool height)
{
	KdShaderManager::Instance().WriteCBFogEnable(distance, height);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 距離フォグの設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::SetDistanceFog(const Math::Vector3& col, float density)
{
	m_parameter.m_distanceFogColor = col;

	m_parameter.m_distanceFogDensity = density;

	m_dirtyFogDist = true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 高さフォグの設定	カラー、上端の高さ、下端の高さ、フォグ開始距離
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::SetheightFog(const Math::Vector3& col, float topValue, float bottomValue, float distance)
{
	m_parameter.m_heightFogColor = col;

	m_parameter.m_heightFogTopValue = topValue;

	m_parameter.m_heightFogBottomValue = bottomValue;
	
	m_parameter.m_heightFogBeginDistance = distance;

	m_dirtyFogHeight = true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 光関連パラメータの書き込み
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::WriteLightParams()
{
	if (m_dirtyLightAmb)
	{
		KdShaderManager::Instance().WriteCBAmbientLight(m_parameter.m_ambientLightColor);
		m_dirtyLightAmb = false;
	}
	if (m_dirtyLightDir)
	{
		KdShaderManager::Instance().WriteCBDirectionalLight(
			m_parameter.m_directionalLightDir, m_parameter.m_directionalLightColor);
		m_dirtyLightDir = false;
	}
	if (!m_pointLights.empty())
	{
		KdShaderManager::Instance().WriteCBPointLight(m_pointLights);
	}

	// 影エリア（中心位置を含めて送る）
	KdShaderManager::Instance().WriteCBShadowArea(m_shadowProj, m_dirLightHeight, m_shadowCenter);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// フォグ関連パラメータの書き込み
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdAmbientController::WriteFogParams()
{
	// 距離フォグ
	if (m_dirtyFogDist)
	{
		KdShaderManager::Instance().WriteCBDistanceFog(m_parameter.m_distanceFogColor,
			m_parameter.m_distanceFogDensity);

		m_dirtyFogDist = false;
	}

	// 高さフォグ
	if (m_dirtyFogHeight)
	{
		KdShaderManager::Instance().WriteCBHeightFog(m_parameter.m_heightFogColor,
			m_parameter.m_heightFogTopValue, m_parameter.m_heightFogBottomValue,
			m_parameter.m_heightFogBeginDistance);

		m_dirtyFogHeight = false;
	}
}
