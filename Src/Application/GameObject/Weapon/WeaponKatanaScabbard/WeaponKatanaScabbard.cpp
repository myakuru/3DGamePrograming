#include "WeaponKatanaScabbard.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"

const uint32_t WeaponKatanaScabbard::TypeID = KdGameObject::GenerateTypeID();

void WeaponKatanaScabbard::Init()
{
	WeaponBase::Init();
	m_scale = { 0.004f,0.004f,0.004f };
}

void WeaponKatanaScabbard::Update()
{
	WeaponBase::Update();
	UpdateMatrix();
}

void WeaponKatanaScabbard::DrawRimLight()
{
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
	KdShaderManager::Instance().m_StandardShader.SetRimLightEnable(true);
	{
		m_rimLightUVOffset += m_rimLightUVOffsetSpeed * Application::Instance().GetUnscaledDeltaTime();

		auto& stdSh = KdShaderManager::Instance().m_StandardShader;
		// 青い武器風
		stdSh.SetRimLight(m_rimLightPower, m_rimLightColor);
		// スパークル用の時間オフセット
		stdSh.SetUVOffset({ m_rimLightUVOffset, -m_rimLightUVOffset });

		stdSh.DrawModel(*m_model, m_swordData.m_weaponMatrix, m_color);

	}
	KdShaderManager::Instance().UndoBlendState();
	KdShaderManager::Instance().m_StandardShader.SetRimLightEnable(false);
}

void WeaponKatanaScabbard::UpdateMatrix()
{
	m_swordHandData.m_weaponRotationMatrix = Math::Matrix::CreateFromYawPitchRoll
	(
		DirectX::XMConvertToRadians(m_swordHandData.m_weaponDeg.y),
		DirectX::XMConvertToRadians(m_swordHandData.m_weaponDeg.x),
		DirectX::XMConvertToRadians(m_swordHandData.m_weaponDeg.z)
	);

	m_swordData.m_weaponScaleMatrix = Math::Matrix::CreateScale(m_swordData.m_scale);

	m_swordHandData.m_weaponBonesMatrix.Translation(m_swordHandData.m_weaponBonesMatrix.Translation());
	Math::Matrix transOffsetMat = Math::Matrix::CreateTranslation(m_katanaOffset);
	m_swordData.m_weaponMatrix = transOffsetMat * m_swordData.m_weaponScaleMatrix * m_swordHandData.m_weaponRotationMatrix * m_swordHandData.m_weaponBonesMatrix * m_swordData.m_playerWorldMatrix;
}

void WeaponKatanaScabbard::ImGuiInspector()
{
	WeaponBase::ImGuiInspector();

	ImGui::Text("Katana Inspector");

	ImGui::Text(U8("刀の角度を変更"));
	ImGui::DragFloat3("deg", &m_swordData.m_weaponDeg.x, 0.1f);
	ImGui::Text(U8("刀の位置を変更"));
	ImGui::DragFloat3("offset", &m_katanaOffset.x, 0.1f);
	ImGui::Text(U8("刀のスケールを変更"));
	ImGui::DragFloat3("scale", &m_swordData.m_scale.x, 0.01f);

	ImGui::Separator();

	if (ImGui::CollapsingHeader(U8("リムライトの設定")))
	{
		ImGui::Text(U8("リムライトの強さを変更"));
		ImGui::DragFloat("RimLightPower", &m_rimLightPower);
		ImGui::Text(U8("リムライトの色を変更"));
		ImGui::ColorEdit3("RimLightColor", &m_rimLightColor.x);
		ImGui::Text(U8("リムライトのUVスクロール速度"));
		ImGui::DragFloat("RimLightUVOffsetSpeed", &m_rimLightUVOffsetSpeed, 0.01f);
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Hand Katana"))
	{
		ImGui::Text(U8("手持ちの刀の角度を変更"));
		ImGui::DragFloat3("handDeg", &m_swordHandData.m_weaponDeg.x, 0.1f);
	}

	UpdateMatrix();
}

void WeaponKatanaScabbard::JsonSave(nlohmann::json& _json) const
{
	WeaponBase::JsonSave(_json);
	_json["rimLightPower"] = m_rimLightPower;
	_json["rimLightColor"] = JSON_MANAGER.VectorToJson(m_rimLightColor);
	_json["rimLightUVOffsetSpeed"] = m_rimLightUVOffsetSpeed;
}

void WeaponKatanaScabbard::JsonInput(const nlohmann::json& _json)
{
	WeaponBase::JsonInput(_json);
	if (_json.contains("rimLightPower")) m_rimLightPower = _json["rimLightPower"].get<float>();
	if (_json.contains("rimLightColor")) m_rimLightColor = JSON_MANAGER.JsonToVector(_json["rimLightColor"]);
	if (_json.contains("rimLightUVOffsetSpeed")) m_rimLightUVOffsetSpeed = _json["rimLightUVOffsetSpeed"].get<float>();
}
