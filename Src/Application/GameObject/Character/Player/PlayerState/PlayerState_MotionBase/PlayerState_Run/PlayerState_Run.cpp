#include "PlayerState_Run.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateServices.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerInputServices.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/CommonAttack/PlayerState_Attack/PlayerState_Attack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_Idle/PlayerState_Idle.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoidFast/PlayerState_ForwardAvoidFast.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoid/PlayerState_ForwardAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_RunEnd/PlayerState_RunEnd.h"

void PlayerState_Run::StateUpdate(Player* _owner)
{
	// 鞘の状態
	PlayerStateServices::UpdateUnsheathed(_owner);

	_owner->UpdateMoveDirectionFromInput();

	Math::Vector3 moveDir = _owner->GetMoveDirection();

	if (moveDir == Math::Vector3::Zero)
	{
		_owner->GetStateMachine().ChangeState<PlayerState_RunEnd>();
	}

	// 攻撃・必殺・回避・Eスキル入力を確認
	PlayerInputServices::TrySpecialAttack<PlayerState_SpecialAttackCutIn>(_owner, this);
	PlayerInputServices::TryAvoid<PlayerState_ForwardAvoidFast, PlayerState_BackWordAvoid, PlayerState_ForwardAvoid>(_owner, this, m_params.rButtonKeyInput);
	PlayerInputServices::TryESkill<PlayerState_ChargeLevel0>(_owner, this);

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_params.lButtonKeyInput = true;
	PlayerInputServices::TryAttack<PlayerState_Attack>(this, m_params.lButtonKeyInput);


	// 方向更新
	_owner->UpdateQuaternion(moveDir);
	_owner->SetIsMoving(moveDir);
}

void PlayerState_Run::OnStartExtra(Player* _owner)
{
	auto anime = _owner->GetAnimeModel()->GetAnimation(GetAnimationName());
	_owner->GetAnimator()->SetAnimation(anime, m_params.blendTime, true);

	if (!m_runSound)
	{
		m_runSound = KdAudioManager::Instance().Play("Asset/Sound/Player/Run.wav", true);
	}
}

void PlayerState_Run::OnUpdateExtra(Player* _owner, float /*deltaTime*/)
{
}

void PlayerState_Run::OnEndExtra(Player* owner)
{
	if (m_runSound)
	{
		m_runSound->Stop();
		m_runSound.reset();
	}
}
