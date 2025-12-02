#include "ResultScore.h"
#include"MyFramework/Manager/JsonManager/JsonManager.h"
#include"Application/main.h"
#include"Application/Scene/SceneManager.h"
#include"Application/GameObject/Camera/TPSCamera/TPSCamera.h"
#include"Application/GameObject/Utility/Time.h"


const uint32_t ResultScore::TypeID = KdGameObject::GenerateTypeID();

void ResultScore::Init()
{
	SelectDraw3dModel::Init();
	m_model_S = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Score/S.gltf");
	m_model_A = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Score/A.gltf");
	m_model_B = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Score/B.gltf");
	m_model_X = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Score/X.gltf");

	m_distance = m_startDistance;
}

void ResultScore::Update()
{
	float deltatime = Application::Instance().GetUnscaledDeltaTime();

	m_time = Time::Instance().GetElapsedTime();

	if (m_time <= 5.0f) return;

	SelectDraw3dModel::Update();

	SceneManager::Instance().GetObjectWeakPtr(m_camera);

	auto _spCamera = m_camera.lock();
	if (!_spCamera) return;

	// カメラのワールド座標を取得
	Math::Vector3 cameraPos = _spCamera->GetMatrix().Translation();

	// カメラの前方向ベクトルを取得
	Math::Vector3 forward = Math::Vector3::TransformNormal(
		Math::Vector3::Forward,
		Math::Matrix::CreateFromQuaternion(_spCamera->GetRotationQuaternion())
	);
	forward.Normalize();

	if (m_distance > 0.0f)
	{
		m_distance -= deltatime * 15.0f; // 徐々に近づく
	}
	else
	{
		m_distance = 0.0f;
	}

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll
	(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	);

	Math::Matrix camRotM = Math::Matrix::CreateFromQuaternion(_spCamera->GetRotationQuaternion());
	Math::Vector3 localOffset = Math::Vector3::TransformNormal(m_position, camRotM);

	// カメラ位置 + カメラ前方向 * 距離 + ローカルオフセット
	m_mWorld.Translation(cameraPos + forward * m_distance + localOffset);
}

void ResultScore::DrawUnLit()
{
	// 最初の5秒間は表示しない
	if (m_time <= m_showTime) return;

	if (SceneManager::Instance().GetScore() == 0)
	{
		m_color = m_S_color;
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model_S, m_mWorld, m_color);
	}
	else if (SceneManager::Instance().GetScore() == 1)
	{
		m_color = m_A_color;
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model_A, m_mWorld, m_color);
	}
	else if (SceneManager::Instance().GetScore() == 2)
	{
		m_color = m_B_color;
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model_B, m_mWorld, m_color);
	}
	else if (SceneManager::Instance().GetScore() == 3)
	{
		m_color = m_X_color;
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model_X, m_mWorld, m_color);
	}
	else
	{
		return; // スコアが0〜3以外の場合は何もしない
	}
}

void ResultScore::ImGuiInspector()
{
	SelectDraw3dModel::ImGuiInspector();

	ImGui::DragFloat(U8("カメラからの距離"), &m_distance);

	ImGui::ColorEdit4(U8("Sの色"), &m_S_color.x);
	ImGui::ColorEdit4(U8("Aの色"), &m_A_color.x);
	ImGui::ColorEdit4(U8("Bの色"), &m_B_color.x);
	ImGui::ColorEdit4(U8("Xの色"), &m_X_color.x);
}

void ResultScore::JsonSave(nlohmann::json& _json) const
{
	SelectDraw3dModel::JsonSave(_json);

	_json["S_color"] = JSON_MANAGER.Vector4ToJson(m_S_color);
	_json["A_color"] = JSON_MANAGER.Vector4ToJson(m_A_color);
	_json["B_color"] = JSON_MANAGER.Vector4ToJson(m_B_color);
	_json["X_color"] = JSON_MANAGER.Vector4ToJson(m_X_color);
}

void ResultScore::JsonInput(const nlohmann::json& _json)
{
	SelectDraw3dModel::JsonInput(_json);
	if (_json.contains("S_color")) m_S_color = JSON_MANAGER.JsonToVector4(_json["S_color"]);
	if (_json.contains("A_color")) m_A_color = JSON_MANAGER.JsonToVector4(_json["A_color"]);
	if (_json.contains("B_color")) m_B_color = JSON_MANAGER.JsonToVector4(_json["B_color"]);
	if (_json.contains("X_color")) m_X_color = JSON_MANAGER.JsonToVector4(_json["X_color"]);
}
