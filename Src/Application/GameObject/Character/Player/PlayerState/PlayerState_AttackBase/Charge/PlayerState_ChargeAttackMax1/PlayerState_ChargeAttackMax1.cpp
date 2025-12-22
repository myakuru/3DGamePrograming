#include "PlayerState_ChargeAttackMax1.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/Charge/PlayerState_ChargeAttackMax2/PlayerState_ChargeAttackMax2.h"

void PlayerState_ChargeAttackMax1::JsonInput(const nlohmann::json& _js)
{
	if (_js.contains("PlayerState_ChargeAttackMax1"))
	{
		PlayerState_AttackBase::JsonInput(_js["PlayerState_ChargeAttackMax1"]);
	}
}

void PlayerState_ChargeAttackMax1::JsonSave(nlohmann::json& _js) const
{
	PlayerState_AttackBase::JsonSave(_js["PlayerState_ChargeAttackMax1"]);
}

void PlayerState_ChargeAttackMax1::OnUpdateAfterDash(Player* _owner)
{
	// 次のチャージに移行
	PlayerInputServices::TryAnimationEnd<PlayerState_ChargeAttackMax2>(_owner, this);
}
