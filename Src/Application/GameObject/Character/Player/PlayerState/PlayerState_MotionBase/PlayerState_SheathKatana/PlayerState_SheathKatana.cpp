#include "PlayerState_SheathKatana.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_Idle/PlayerState_Idle.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/CommonAttack/PlayerState_Attack/PlayerState_Attack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoidFast/PlayerState_ForwardAvoidFast.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoid/PlayerState_ForwardAvoid.h"

void PlayerState_SheathKatana::OnUpdateExtra(Player* owner, float /*deltaTime*/)
{
	// 納刀中は移動させない
	owner->SetIsMoving(Math::Vector3::Zero);
	owner->SetMoveDirection(Math::Vector3::Zero);

	// 鞘の状態更新（Idle/Run同様の共通処理）
	PlayerStateServices::UpdateUnsheathed(owner);

	// 途中入力の受け付け（必要に応じて制限可能）
	PlayerInputServices::TrySpecialAttack<PlayerState_SpecialAttackCutIn>(owner, this);
	PlayerInputServices::TryAvoid<PlayerState_ForwardAvoidFast, PlayerState_BackWordAvoid, PlayerState_ForwardAvoid>(owner, this, m_params.rButtonKeyInput);
	PlayerInputServices::TryESkill<PlayerState_ChargeLevel0>(owner, this);

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_params.lButtonKeyInput = true;
	PlayerInputServices::TryAttack<PlayerState_Attack>(this, m_params.lButtonKeyInput);

	// アニメ時間で Idle へ（Idle/Runに合わせ changeStateTime を利用）
	const float animeTime = owner->GetAnimator()->GetPlayProgress();
	if (animeTime >= m_params.changeStateTime)
	{
		owner->GetStateMachine().ChangeState<PlayerState_Idle>();
	}
}