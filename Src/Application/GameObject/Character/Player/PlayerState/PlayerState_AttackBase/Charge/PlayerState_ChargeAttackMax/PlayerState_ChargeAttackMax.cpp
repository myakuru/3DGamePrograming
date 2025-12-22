#include "PlayerState_ChargeAttackMax.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/Charge/PlayerState_ChargeAttackMax1/PlayerState_ChargeAttackMax1.h"
#include "Application/GameObject/Character/AfterImage/AfterImage.h"

void PlayerState_ChargeAttackMax::ExposeParametersImGui()
{
	m_params.ExposeImGui();
	ImGui::DragFloat("SearchEnemyRadius", &m_params.searchEnemyRadius, 0.1f, 0.0f, 20.0f);
	// 残像設定
	ImGui::DragInt("afterImageMax", &m_params.afterImageMax, 1, 0, 20);
	ImGui::DragFloat("afterImageInterval", &m_params.afterImageInterval, 0.01f, 0.0f, 1.0f);
	ImGui::ColorEdit4("afterImageColor", &m_params.afterImageColor.x);
}

void PlayerState_ChargeAttackMax::JsonInput(const nlohmann::json& _js)
{
	if (!_js.contains("PlayerState_ChargeAttackMax")) return;

	PlayerState_AttackBase::JsonInput("PlayerState_ChargeAttackMax");

	const auto& stateNode = _js["PlayerState_ChargeAttackMax"];
	if (stateNode.contains("Player")) m_params.LoadJson(stateNode["Player"]);
	if (stateNode.contains("SearchEnemyRadius")) m_params.searchEnemyRadius = stateNode["SearchEnemyRadius"].get<float>();
	// 残像設定
	if (stateNode.contains("afterImageMax")) m_params.afterImageMax = stateNode["afterImageMax"].get<int>();
	if (stateNode.contains("afterImageInterval")) m_params.afterImageInterval = stateNode["afterImageInterval"].get<float>();
	if (stateNode.contains("afterImageColor")) m_params.afterImageColor = JSON_MANAGER.JsonToVector4(stateNode["afterImageColor"]);
}

void PlayerState_ChargeAttackMax::JsonSave(nlohmann::json& _js) const
{
	PlayerState_AttackBase::JsonSave(_js["PlayerState_ChargeAttackMax"]);

	_js["PlayerState_ChargeAttackMax"]["SearchEnemyRadius"] = m_params.searchEnemyRadius;
	// 残像設定
	_js["PlayerState_ChargeAttackMax"]["afterImageMax"] = m_params.afterImageMax;
	_js["PlayerState_ChargeAttackMax"]["afterImageInterval"] = m_params.afterImageInterval;
	_js["PlayerState_ChargeAttackMax"]["afterImageColor"] = JSON_MANAGER.Vector4ToJson(m_params.afterImageColor);
}

void PlayerState_ChargeAttackMax::OnStartExtra(Player* _owner)
{
	// 残像設定
	if (auto afterImage = _owner->GetAfterImage())
	{
		afterImage->AddAfterImage(true, m_params.afterImageMax, m_params.afterImageInterval, m_params.afterImageColor);
	}

	// ガードブレイク状態にする
	_owner->SetGuardBreak(true);
}

void PlayerState_ChargeAttackMax::OnUpdateAfterDash(Player* _owner)
{
	// 次のチャージに移行
	PlayerInputServices::TryAnimationEnd<PlayerState_ChargeAttackMax1>(_owner, this);
}