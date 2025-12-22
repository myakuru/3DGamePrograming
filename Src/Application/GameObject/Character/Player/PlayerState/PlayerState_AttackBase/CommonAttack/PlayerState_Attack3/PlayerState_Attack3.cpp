#include "PlayerState_Attack3.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/CommonAttack/PlayerState_Attack4/PlayerState_Attack4.h"

void PlayerState_Attack3::OnUpdateAfterDash(Player* _owner)
{
	// 攻撃入力受付
	PlayerInputServices::TryAttack<PlayerState_Attack4>(this, m_lButtonKeyInput);

	// アニメーション終了後、納刀モーションに自動遷移
	PlayerInputServices::TrySheath<PlayerState_SheathKatana>(_owner, this);
}

void PlayerState_Attack3::JsonInput(const nlohmann::json& _js)
{
	if (_js.contains("PlayerState_Attack3"))
	{
		PlayerState_AttackBase::JsonInput(_js["PlayerState_Attack3"]);
	}
}

void PlayerState_Attack3::JsonSave(nlohmann::json& _js) const
{
	PlayerState_AttackBase::JsonSave(_js["PlayerState_Attack3"]);
}
