#include "MapGard.h"
#include"../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../../Scene/SceneManager.h"
#include"../../../Character/Player/Player.h"

const uint32_t MapGard::TypeID = KdGameObject::GenerateTypeID();

void MapGard::Init()
{
	SelectDraw3dPolygon::Init();
	SceneManager::Instance().GetObjectWeakPtr(m_camera);

	m_polygon->SetMaterial("Asset/Textures/GameUI/Collison.png");

	SceneManager::Instance().GetObjectWeakPtr(m_player);

	m_scale = { 0.15f,0.8f,1.0f };

	m_position.y = 10.7f;

}

void MapGard::Update()
{
	// プレイヤーとの距離（実距離で比較するように修正）
	if (auto player = m_player.lock(); player)
	{
		m_playerDistance = (player->GetPos() - m_position).Length();
	}

	if (m_playerDistance < 5.0f)
	{
		m_color = { 0.8f,0.8f,0.8f,1.0f };
	}
	else if (m_playerDistance < 8.0f)
	{
		m_color = { 0.8f,0.8f,0.8f,0.8f };
	}
	else
	{
		m_color = { 0.8f,0.8f,0.8f,0.5f };
	}

	// 常に微細に揺れる（位置へ加算しない）
	Math::Vector3 jitter = {
		KdRandom::GetFloat(-0.1f, 0.1f),
		KdRandom::GetFloat(-0.1f, 0.1f),
		KdRandom::GetFloat(-0.1f, 0.1f)
	};
	m_offset = jitter; // 必要なら保持。使用しないなら代入を省略可。

	// カメラが存在するなら
	if (auto spCamera = m_camera.lock(); spCamera)
	{
		Math::Vector3 camPos = spCamera->GetCamera()->GetCameraMatrix().Translation();

		// ベース位置 + 微細オフセット
		Math::Vector3 worldPos = m_position + (m_offset * 0.01f);

		// ビルボード行列（位置は worldPos を使用）
		Math::Matrix rot = Math::Matrix::CreateBillboard
		(
			worldPos,                   // オブジェクトの位置（描画用）
			camPos,                     // カメラの位置
			Math::Vector3::UnitY        // カメラの上方向
		);


		// 最終的なワールド行列
		m_mWorld = Math::Matrix::CreateScale(m_scale) * rot;
	}
}

void MapGard::DrawToon()
{
	KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZWriteDisable);
	SelectDraw3dPolygon::DrawToon();
	KdShaderManager::Instance().UndoDepthStencilState();
}
