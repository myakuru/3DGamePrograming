#include "PlayerState_AvoidAttack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SheathKatana/PlayerState_SheathKatana.h"

void PlayerState_AvoidAttack::OnUpdateAfterDash(Player* _owner)
{
	// アニメーション終了後、納刀モーションに自動遷移
	PlayerInputServices::TrySheath<PlayerState_SheathKatana>(_owner, this);
}