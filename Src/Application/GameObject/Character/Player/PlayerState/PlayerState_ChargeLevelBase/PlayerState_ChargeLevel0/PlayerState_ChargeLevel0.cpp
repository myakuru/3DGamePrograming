#include "PlayerState_ChargeLevel0.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevel1/PlayerState_ChargeLevel1.h"

void PlayerState_ChargeLevel0::OnUpdateAfterDash(Player* _owner)
{
	PlayerInputServices::TryAnimationEnd<PlayerState_ChargeLevel1>(_owner,this);
}