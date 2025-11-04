#include "TitleCamera.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"

const uint32_t TitleCamera::TypeID = KdGameObject::GenerateTypeID();

void TitleCamera::Init()
{
	CameraBase::Init();
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 2.f, -2.0f);
	GetCursorPos(&m_FixMousePos);

	m_time = 0.0f;
}

void TitleCamera::PostUpdate()
{
	if (!SceneManager::Instance().m_sceneCamera)
	{
		// カメラの回転
		UpdateRotateByMouse();
		m_mRotation = GetRotationMatrix();
		UpdateMoveKey();

	}

	float deltaTime = Application::Instance().GetDeltaTime();

	//	カメラを行ったり来たりラープさせる
	m_time += deltaTime * m_moveSpeed;
	// 0～2をループさせる
	float u = std::fmod(m_time, 2.0f);
	// 0～1～0に変換
	if (u > 1.0f) u = 2.0f - u; // 0～1～0


	m_position = Math::Vector3::Lerp(m_startPos, m_endPos, u);
	

	m_mWorld = m_mLocalPos * m_mRotation;
	m_mWorld.Translation(m_position);
	m_spCamera->SetCameraMatrix(m_mWorld);
}

void TitleCamera::ImGuiInspector()
{
	CameraBase::ImGuiInspector();
	ImGui::DragFloat(U8("ラープ時間"), &m_moveSpeed, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3(U8("開始位置"), &m_startPos.x, 0.1f);
	ImGui::DragFloat3(U8("終了位置"), &m_endPos.x, 0.1f);

}

void TitleCamera::JsonSave(nlohmann::json& _json) const
{
	CameraBase::JsonSave(_json);
	_json["m_startPos"] = JSON_MANAGER.VectorToJson(m_startPos);
	_json["m_endPos"] = JSON_MANAGER.VectorToJson(m_endPos);
	_json["m_moveSpeed"] = m_moveSpeed;
}

void TitleCamera::JsonInput(const nlohmann::json& _json)
{
	CameraBase::JsonInput(_json);
	if (_json.contains("m_startPos")) m_startPos = JSON_MANAGER.JsonToVector(_json["m_startPos"]);
	if (_json.contains("m_endPos")) m_endPos = JSON_MANAGER.JsonToVector(_json["m_endPos"]);
	if (_json.contains("m_moveSpeed")) m_moveSpeed = _json["m_moveSpeed"].get<float>();

}
