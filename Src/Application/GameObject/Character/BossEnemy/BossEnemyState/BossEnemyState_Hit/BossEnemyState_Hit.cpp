#include "BossEnemyState_Hit.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../BossEnemyState_Attack_L/BossEnemyState_Attack_L.h"
#include"../BossEnemyAI.h" // 追加

void BossEnemyState_Hit::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Hit");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(60.0f);

	// 累積ヒット回数は Enemy 本体で管理
	m_bossEnemy->IncrementHitCount();

	// 10回以上で無敵
	if (m_bossEnemy->GetHitCount() >= 10)
	{
		m_bossEnemy->SetInvincible(true);
		m_bossEnemy->ResetHitCount();
	}
}

void BossEnemyState_Hit::StateUpdate()
{
	// 無敵中：これまでAttack_L固定だったが、AIに委譲して多様化
	if (m_bossEnemy->GetInvincible())
	{
		auto next = BossEnemyAI::DecideNext(m_bossEnemy);
		m_bossEnemy->ChangeState(next);
		return;
	}

	// アニメーション終了：Idle固定だったが、AIに委譲
	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto next = BossEnemyAI::DecideNext(m_bossEnemy);
		m_bossEnemy->ChangeState(next);
		return;
	}

	if (m_time < 0.2f)
	{
		const float dashSpeed = -0.5f;
		m_bossEnemy->SetIsMoving(m_attackDirection * dashSpeed);
	}
	else
	{
		m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
	}
}

void BossEnemyState_Hit::StateEnd()
{
}