#include "PlayerState_Hit.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Attack/PlayerState_Attack.h"

#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../../main.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_Hit::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Hit");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	PlayerStateBase::StateStart();

	m_time = 0.0f;
}

void PlayerState_Hit::StateUpdate()
{
	m_player->SetAnimeSpeed(100.0f);

	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();

		m_maxAnimeTime = m_player->GetAnimator()->GetMaxAnimationTime();
	}

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto idleState = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(idleState);
		return;
	}

	if (m_animeTime >= 0.5f)
	{
		// 回避入力処理
		if (UpdateMoveAvoidInput()) return;
	}

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	PlayerStateBase::StateUpdate();

	UpdateKatanaPos();

	if (m_animeTime >= 0.9f)
	{
		// 押された瞬間
		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_LButtonkeyInput = true; // 判定開始

		// 攻撃入力受付
		if (UpdateAttackInput<PlayerState_Attack>()) return;
	}


	float deltaTime = Application::Instance().GetDeltaTime();

	if (m_time < 0.2f)
	{
		float dashSpeed = -0.6f;
		m_player->SetIsMoving(m_attackDirection * dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		m_player->SetIsMoving(Math::Vector3::Zero);
	}
}

void PlayerState_Hit::StateEnd()
{
	PlayerStateBase::StateEnd();
	m_player->SetHitCheck(false);
}
