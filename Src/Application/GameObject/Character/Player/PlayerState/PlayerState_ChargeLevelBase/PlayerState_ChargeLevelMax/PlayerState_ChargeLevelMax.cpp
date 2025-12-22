#include "PlayerState_ChargeLevelMax.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/Charge/PlayerState_ChargeAttackMax/PlayerState_ChargeAttackMax.h"

void PlayerState_ChargeLevelMax::OnUpdateAfterDash(Player* _owner)
{
	// アニメーション終了後、納刀モーションに自動遷移
	PlayerInputServices::TryAnimationEnd<PlayerState_ChargeAttackMax>(_owner,this);
}