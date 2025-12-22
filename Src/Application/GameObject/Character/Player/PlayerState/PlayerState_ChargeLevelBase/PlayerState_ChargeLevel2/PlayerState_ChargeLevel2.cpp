#include "PlayerState_ChargeLevel2.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevelMax/PlayerState_ChargeLevelMax.h"

void PlayerState_ChargeLevel2::OnUpdateAfterDash(Player* _owner)
{
	// 攻撃入力受付
	PlayerInputServices::TryAnimationEnd<PlayerState_ChargeLevelMax>(_owner, this);
}