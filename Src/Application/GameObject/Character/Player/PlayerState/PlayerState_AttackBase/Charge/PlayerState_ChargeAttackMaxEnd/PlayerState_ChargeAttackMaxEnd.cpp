#include "PlayerState_ChargeAttackMaxEnd.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include "Application/GameObject/Character/AfterImage/AfterImage.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"

void PlayerState_ChargeAttackMaxEnd::JsonInput(const nlohmann::json& _js)
{
	if (_js.contains("PlayerState_ChargeAttackMaxEnd"))
	{
		auto& playerNode = _js["PlayerState_ChargeAttackMaxEnd"];

		PlayerState_AttackBase::JsonInput(_js["PlayerState_ChargeAttackMaxEnd"]);
		// 残像設定
		if (playerNode.contains("afterImageMax")) m_params.afterImageMax = playerNode["afterImageMax"].get<int>();
		if (playerNode.contains("afterImageInterval")) m_params.afterImageInterval = playerNode["afterImageInterval"].get<float>();
		if (playerNode.contains("afterImageColor")) m_params.afterImageColor = JSON_MANAGER.JsonToVector4(playerNode["afterImageColor"]);
	}
}

void PlayerState_ChargeAttackMaxEnd::JsonSave(nlohmann::json& _js) const
{
	PlayerState_AttackBase::JsonSave(_js["PlayerState_ChargeAttackMaxEnd"]);

	auto& stateNode = _js["PlayerState_ChargeAttackMaxEnd"];

	// 残像設定
	stateNode["Player"]["afterImageMax"] = m_params.afterImageMax;
	stateNode["Player"]["afterImageInterval"] = m_params.afterImageInterval;
	stateNode["Player"]["afterImageColor"] = JSON_MANAGER.Vector4ToJson(m_params.afterImageColor);
}

void PlayerState_ChargeAttackMaxEnd::OnUpdateAfterDash(Player* _owner)
{
	PlayerInputServices::TrySheath<PlayerState_SheathKatana>(_owner, this);
}

void PlayerState_ChargeAttackMaxEnd::OnEndExtra(Player* _owner)
{
	// 残像のリセット
	if (auto afterImage = _owner->GetAfterImage())
	{
		afterImage->AddAfterImage();
	}

	// 無敵状態解除
	_owner->SetInvincible(false);

	// ガードブレイク状態解除
	_owner->SetGuardBreak(false);
}
