#include "PlayerState_Idle.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/CommonAttack/PlayerState_Attack/PlayerState_Attack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoidFast/PlayerState_ForwardAvoidFast.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoid/PlayerState_ForwardAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_Run/PlayerState_Run.h"

void PlayerState_Idle::OnStartExtra(Player* _owner)
{
	auto anime = _owner->GetAnimeModel()->GetAnimation(GetAnimationName());
	_owner->GetAnimator()->SetAnimation(anime, m_params.blendTime, true);

	const bool bossAppeared = SceneManager::Instance().IsBossAppear();
	if (auto camera = _owner->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(bossAppeared ? m_cameraBossTargetOffset : m_cameraTargetOffset);
	}
}

void PlayerState_Idle::OnUpdateExtra(Player* owner, float /*deltaTime*/)
{
	// 鞘の状態更新（旧 StateUpdate 冒頭）
	PlayerStateServices::UpdateUnsheathed(owner);

	// W+S同時、D+A同時の場合は移動しない（旧ロジック維持）
	if ((KeyboardManager::GetInstance().IsKeyPressed('W') && KeyboardManager::GetInstance().IsKeyPressed('S')) ||
		(KeyboardManager::GetInstance().IsKeyPressed('D') && KeyboardManager::GetInstance().IsKeyPressed('A')))
	{
		owner->SetIsMoving(Math::Vector3::Zero);
		owner->SetMoveDirection(Math::Vector3::Zero);
		return;
	}

	// キーが押されたらRunへ（旧ロジック維持）
	if (KeyboardManager::GetInstance().IsKeyPressed('W') ||
		KeyboardManager::GetInstance().IsKeyPressed('A') ||
		KeyboardManager::GetInstance().IsKeyPressed('S') ||
		KeyboardManager::GetInstance().IsKeyPressed('D'))
	{
		PlayerInputServices::TryChange<PlayerState_Run>(this);
	}

	// 入力：必殺技・回避・Eスキル・攻撃
	PlayerInputServices::TrySpecialAttack<PlayerState_SpecialAttackCutIn>(owner, this);
	PlayerInputServices::TryAvoid<PlayerState_ForwardAvoidFast, PlayerState_BackWordAvoid, PlayerState_ForwardAvoid>(owner, this, m_params.rButtonKeyInput);
	PlayerInputServices::TryESkill<PlayerState_ChargeLevel0>(owner, this);

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_params.lButtonKeyInput = true;
	PlayerInputServices::TryAttack<PlayerState_Attack>(this, m_params.lButtonKeyInput);
	

	// Idleは常に移動量リセット（旧ロジック維持）
	owner->SetIsMoving(Math::Vector3::Zero);
}

