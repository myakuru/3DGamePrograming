#include "ResultScore.h"
#include"../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../main.h"
#include"../../Scene/SceneManager.h"
#include"../Camera/TPSCamera/TPSCamera.h"
#include"../Utility/Time.h"
const uint32_t ResultScore::TypeID = KdGameObject::GenerateTypeID();

void ResultScore::Init()
{
	SelectDraw3dModel::Init();
	m_model_S = KdAssets::Instance().m_modeldatas.GetData("Asset/3DAssets/Score/S.gltf");
	m_model_A = KdAssets::Instance().m_modeldatas.GetData("Asset/3DAssets/Score/A.gltf");
	m_model_B = KdAssets::Instance().m_modeldatas.GetData("Asset/3DAssets/Score/B.gltf");
	m_model_X = KdAssets::Instance().m_modeldatas.GetData("Asset/3DAssets/Score/X.gltf");

	KdShaderManager::Instance().m_StandardShader.SetGradientColor(m_gradientColor);

	m_distance = 20.0f;
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

	// m_position をカメラローカルのオフセットとして使用する場合はカメラ回転を適用
	Math::Matrix camRotM = Math::Matrix::CreateFromQuaternion(_spCamera->GetRotationQuaternion());
	Math::Vector3 localOffset = Math::Vector3::TransformNormal(m_position, camRotM);

	// カメラ位置 + カメラ前方向 * 距離 + ローカルオフセット
	m_mWorld.Translation(cameraPos + forward * m_distance + localOffset);
}

void ResultScore::DrawGradation()
{
	if (m_time <= 5.0f) return;

	if (SceneManager::Instance().GetScore() == 0)
	{
		m_color = { 1.0f, 0.43f, 0.04f, 1.0f };
		m_gradientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model_S, m_mWorld, m_color);
	}
	else if (SceneManager::Instance().GetScore() == 1)
	{
		m_color = { 0.71f, 0.24f, 1.0f, 1.0f };
		m_gradientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model_A, m_mWorld, m_color);
	}
	else if (SceneManager::Instance().GetScore() == 2)
	{
		m_color = { 0.19f, 0.62f, 1.0f, 1.0f };
		m_gradientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model_B, m_mWorld, m_color);
	}
	else if (SceneManager::Instance().GetScore() == 3)
	{
		m_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_gradientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
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
	ImGui::ColorEdit3(U8("グラデーションのカラー"), &m_gradientColor.x);
	KdShaderManager::Instance().m_StandardShader.SetGradientColor(m_gradientColor);

	ImGui::DragFloat(U8("カメラからの距離"), &m_distance);
}

void ResultScore::JsonSave(nlohmann::json& _json) const
{
	SelectDraw3dModel::JsonSave(_json);
	_json["gradientColor"] = JSON_MANAGER.Vector4ToJson(m_gradientColor);
}

void ResultScore::JsonInput(const nlohmann::json& _json)
{
	SelectDraw3dModel::JsonInput(_json);
	if (_json.contains("gradientColor"))
	{
		m_gradientColor = JSON_MANAGER.JsonToVector4(_json["gradientColor"]);
		KdShaderManager::Instance().m_StandardShader.SetGradientColor(m_gradientColor);
	}
}
