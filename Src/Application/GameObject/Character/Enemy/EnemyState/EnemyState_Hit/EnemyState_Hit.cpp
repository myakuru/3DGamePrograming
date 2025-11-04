#include "EnemyState_Hit.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../../../Player/Player.h"
#include"../EnemyState_Attack/EnemyState_Attack.h"

void EnemyState_Hit::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Hit");
	m_enemy->GetAnimator()->SetAnimation(anime, 0.25f, false);

	m_enemy->SetAnimeSpeed(60.0f);

	// 累積ヒット回数は Enemy 本体で管理
	m_enemy->IncrementHitCount();

	// 10回以上で無敵
	if (m_enemy->GetHitCount() >= 10)
	{
		m_enemy->SetInvincible(true);
		m_enemy->ResetHitCount();
	}

}

void EnemyState_Hit::StateUpdate()
{
	if (m_enemy->GetInvincible())
	{
		// 無敵状態ならAttackへ
		auto spIdleState = std::make_shared<EnemyState_Attack>();
		m_enemy->ChangeState(spIdleState);
		return;
	}


	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		// Idleステートに移行
		auto spIdleState = std::make_shared<EnemyState_Idle>();
		m_enemy->ChangeState(spIdleState);
		return;
	}

	if (m_time < 0.2f)
	{
		const float dashSpeed = -0.5f;
		m_enemy->SetIsMoving(m_attackDirection * dashSpeed);
	}
	else
	{
		m_enemy->SetIsMoving(Math::Vector3::Zero);
	}
}

void EnemyState_Hit::StateEnd()
{
}
