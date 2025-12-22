#include "PlayerState_ChargeAttackMax2.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/Charge/PlayerState_ChargeAttackMaxEnd/PlayerState_ChargeAttackMaxEnd.h"

void PlayerState_ChargeAttackMax2::JsonInput(const nlohmann::json& _js)
{
	if (_js.contains("PlayerState_ChargeAttackMax2"))
	{
		PlayerState_AttackBase::JsonInput(_js["PlayerState_ChargeAttackMax2"]);
	}
}

void PlayerState_ChargeAttackMax2::JsonSave(nlohmann::json& _js) const
{
	PlayerState_AttackBase::JsonSave(_js["PlayerState_ChargeAttackMax2"]);
}

void PlayerState_ChargeAttackMax2::OnUpdateAfterDash(Player* _owner)
{
	// 次のチャージに移行
	PlayerInputServices::TryAnimationEnd<PlayerState_ChargeAttackMaxEnd>(_owner, this);
}
