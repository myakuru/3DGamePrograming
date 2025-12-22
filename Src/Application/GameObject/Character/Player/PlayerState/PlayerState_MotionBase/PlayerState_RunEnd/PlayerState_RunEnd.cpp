#include "PlayerState_RunEnd.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_Idle/PlayerState_Idle.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/CommonAttack/PlayerState_Attack/PlayerState_Attack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoidFast/PlayerState_ForwardAvoidFast.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoid/PlayerState_ForwardAvoid.h"

void PlayerState_RunEnd::OnStartExtra(Player* owner)
{
	// RunEnd開始時は移動を止める
	owner->SetIsMoving(Math::Vector3::Zero);
	owner->SetMoveDirection(Math::Vector3::Zero);
	// 必要ならカメラ・エフェクト初期化など
}

void PlayerState_RunEnd::OnUpdateExtra(Player* owner, float /*deltaTime*/)
{
	// 納刀/抜刀等の見た目更新（必要なら）
	PlayerStateServices::UpdateUnsheathed(owner);

	// RunEnd中は移動させない
	owner->SetIsMoving(Math::Vector3::Zero);
	owner->SetMoveDirection(Math::Vector3::Zero);

	// 入力受付（必要なら制限可能。RunEnd中も同様に受け付ける例）
	PlayerInputServices::TrySpecialAttack<PlayerState_SpecialAttackCutIn>(owner, this);
	PlayerInputServices::TryAvoid<PlayerState_ForwardAvoidFast, PlayerState_BackWordAvoid, PlayerState_ForwardAvoid>(owner, this, m_params.rButtonKeyInput);
	PlayerInputServices::TryESkill<PlayerState_ChargeLevel0>(owner, this);

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_params.lButtonKeyInput = true;
	PlayerInputServices::TryAttack<PlayerState_Attack>(this, m_params.lButtonKeyInput);

	// アニメ時間で Idle に遷移
	const float animeTime = owner->GetAnimator()->GetPlayProgress();
	if (animeTime >= m_params.changeStateTime)
	{
		owner->GetStateMachine().ChangeState<PlayerState_Idle>();
	}
}