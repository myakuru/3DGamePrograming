#include "WeaponBase.h"
#include"../../main.h"
#include"../../../MyFramework/Manager/JsonManager/JsonManager.h"

const uint32_t WeaponBase::TypeID = KdGameObject::GenerateTypeID();

void WeaponBase::Init()
{
	KdGameObject::Init();
}

void WeaponBase::DrawToon()
{
	if (m_bDrawToon) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_swordData.m_weaponMatrix);
}

void WeaponBase::DrawLit()
{
	if (m_bDrawLit) KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_swordData.m_weaponMatrix,m_color);
}

void WeaponBase::Update()
{
	KdGameObject::Update();
}

void WeaponBase::ImGuiInspector()
{
	SelectDraw3dModel::ImGuiInspector();
}

void WeaponBase::JsonSave(nlohmann::json& _json) const
{
	SelectDraw3dModel::JsonSave(_json);
	_json["weaponDeg"] = JSON_MANAGER.VectorToJson(m_swordData.m_weaponDeg);
	_json["katanaOffset"] = JSON_MANAGER.VectorToJson(m_katanaOffset);
	_json["katanaHandOffset"] = JSON_MANAGER.VectorToJson(m_katanaHandOffset);
	_json["scale"] = JSON_MANAGER.VectorToJson(m_swordData.m_scale);
	_json["handWeaponDeg"] = JSON_MANAGER.VectorToJson(m_swordHandData.m_weaponDeg);
}

void WeaponBase::JsonInput(const nlohmann::json& _json)
{
	SelectDraw3dModel::JsonInput(_json);
	if (_json.contains("weaponDeg")) m_swordData.m_weaponDeg = JSON_MANAGER.JsonToVector(_json["weaponDeg"]);
	if (_json.contains("katanaOffset")) m_katanaOffset = JSON_MANAGER.JsonToVector(_json["katanaOffset"]);
	if (_json.contains("katanaHandOffset")) m_katanaHandOffset = JSON_MANAGER.JsonToVector(_json["katanaHandOffset"]);
	if (_json.contains("scale")) m_swordData.m_scale = JSON_MANAGER.JsonToVector(_json["scale"]);
	if (_json.contains("handWeaponDeg")) m_swordHandData.m_weaponDeg = JSON_MANAGER.JsonToVector(_json["handWeaponDeg"]);
}

Math::Matrix WeaponBase::ToRotationMatrix(const Math::Vector3& deg)
{
	return Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(deg.y),
		DirectX::XMConvertToRadians(deg.x),
		DirectX::XMConvertToRadians(deg.z));
}

Math::Matrix WeaponBase::ComposeWeaponMatrix(const SwordData& trsSrc, const SwordData& attachSrc, const Math::Vector3& offset, const Math::Matrix& ownerWorld) const
{
	Math::Matrix rot = ToRotationMatrix(trsSrc.m_weaponDeg);
	Math::Matrix scl = Math::Matrix::CreateScale(trsSrc.m_scale);

	// ボーンの回転・拡縮成分を無視して平行移動のみ使用
	Math::Matrix bone = attachSrc.m_weaponBonesMatrix;
	bone.Translation(attachSrc.m_weaponBonesMatrix.Translation());

	Math::Matrix transOffset = Math::Matrix::CreateTranslation(offset);

	return transOffset * scl * rot * bone * ownerWorld;
}
