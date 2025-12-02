#include "Winner.h"
#include"../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../main.h"
#include"../../Scene/SceneManager.h"
#include"../Camera/PlayerCamera/PlayerCamera.h"

const uint32_t Winner::TypeID = KdGameObject::GenerateTypeID();

void Winner::Init()
{
	m_polygon->SetMaterial("Asset/Textures/GameUI/WINNER.png");
}

void Winner::Update()
{

	if (SceneManager::Instance().m_gameClear)
	{
		SceneManager::Instance().GetObjectWeakPtr(m_camera);

		auto _spCamera = m_camera.lock();
		if (!_spCamera) return;

		// カメラのワールド座標を取得
		Math::Vector3 cameraPos = _spCamera->GetCamera()->GetCameraMatrix().Translation();
		Math::Matrix cameraForward = _spCamera->GetRotationYMatrix();
		Math::Vector3 cameraDir = cameraForward.Backward();

		m_position = cameraPos + cameraDir * m_distance;

		// --- ビルボード処理 ---
		Math::Matrix cameraRot = _spCamera->GetRotationYMatrix();

		m_mWorld =
			Math::Matrix::CreateScale(m_scale) *
			cameraRot *
			Math::Matrix::CreateTranslation(m_position);
	}
}

void Winner::DrawLit()
{
	if (SceneManager::Instance().m_gameClear)
	{
		KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZDisable);
		SelectDraw3dPolygon::DrawLit();
		KdShaderManager::Instance().UndoDepthStencilState();
	}
}

void Winner::ImGuiInspector()
{
	SelectDraw3dPolygon::ImGuiInspector();
	ImGui::Text(U8("Winnerの設定"));
	ImGui::Checkbox(U8("ゲームクリア時に表示"), &SceneManager::Instance().m_gameClear);
	ImGui::DragFloat(U8("カメラからの距離"), &m_distance, 0.1f, 0.0f, 100.0f);
}

void Winner::JsonSave(nlohmann::json& _json) const
{
	SelectDraw3dPolygon::JsonSave(_json);
	_json["distance"] = m_distance; // カメラからの距離を保存
}

void Winner::JsonInput(const nlohmann::json& _json)
{
	SelectDraw3dPolygon::JsonInput(_json);
	if (_json.contains("distance")) m_distance = _json["distance"].get<float>();
}
