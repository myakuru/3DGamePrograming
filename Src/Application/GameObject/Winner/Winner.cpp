#include "Winner.h"
#include"../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../main.h"
#include"../../Scene/SceneManager.h"
#include"../Camera/PlayerCamera/PlayerCamera.h"

const uint32_t Winner::TypeID = KdGameObject::GenerateTypeID();

void Winner::Init()
{
	KdShaderManager::Instance().m_StandardShader.SetGradientColor(m_gradientColor);
	//m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameUI/WINNER.png");
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

void Winner::DrawGradation()
{
	if (SceneManager::Instance().m_gameClear)
	{
		SelectDraw3dPolygon::DrawGradation();
	}
}

void Winner::ImGuiInspector()
{
	SelectDraw3dPolygon::ImGuiInspector();
	ImGui::ColorEdit4(U8("グラデーションのカラー"), &m_gradientColor.x);
	KdShaderManager::Instance().m_StandardShader.SetGradientColor(m_gradientColor);
	ImGui::Text(U8("Winnerの設定"));
	ImGui::Checkbox(U8("ゲームクリア時に表示"), &SceneManager::Instance().m_gameClear);
	ImGui::DragFloat(U8("カメラからの距離"), &m_distance, 0.1f, 0.0f, 100.0f);
}

void Winner::JsonSave(nlohmann::json& _json) const
{
	SelectDraw3dPolygon::JsonSave(_json);
	_json["gradientColor"] = JSON_MANAGER.Vector4ToJson(m_gradientColor);
	_json["distance"] = m_distance; // カメラからの距離を保存
}

void Winner::JsonInput(const nlohmann::json& _json)
{
	if (_json.contains("gradientColor"))
	{
		m_gradientColor = JSON_MANAGER.JsonToVector4(_json["gradientColor"]);
		KdShaderManager::Instance().m_StandardShader.SetGradientColor(m_gradientColor);
	}
	if (_json.contains("distance"))
	{
		m_distance = _json["distance"].get<float>();
	}
	SelectDraw3dPolygon::JsonInput(_json);
}
