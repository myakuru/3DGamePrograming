#include "PlayerState_Attack4.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/CommonAttack/PlayerState_Attack1/PlayerState_Attack1.h"

void PlayerState_Attack4::OnUpdateAfterDash(Player* _owner)
{
	// 攻撃入力受付
	PlayerInputServices::TryAttack<PlayerState_Attack1>(this, m_lButtonKeyInput);

	// アニメーション終了後、納刀モーションに自動遷移
	PlayerInputServices::TrySheath<PlayerState_SheathKatana>(_owner, this);
}
