#include "PlayerState_ChargeLevel1.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevel2/PlayerState_ChargeLevel2.h"

void PlayerState_ChargeLevel1::OnUpdateAfterDash(Player* _owner)
{
	// 攻撃入力受付
	PlayerInputServices::TryAnimationEnd<PlayerState_ChargeLevel2>(_owner, this);
}