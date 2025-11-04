#include "BossEnemyState_Attack_R.h"
#include"../../../Player/Player.h"
#include"../BossEnemyState_Run/BossEnemyState_Run.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../BossEnemyAI.h" 

void BossEnemyState_Attack_R::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Attack_R");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(60.0f);

	// 当たり判定リセット
	m_bossEnemy->ResetAttackCollision();

	// 近接CDと直前行動をセット
	m_bossEnemy->SetMeleeCooldown(1.0f);
	m_bossEnemy->SetLastAction(BossEnemy::ActionType::AttackR);
}

void BossEnemyState_Attack_R::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// アニメーション再生時間を取得
	m_animeTime = m_bossEnemy->GetAnimator()->GetPlayProgress();

	KdDebugGUI::Instance().AddLog("EnemyAttackAnimeTime:%f", m_animeTime);
	KdDebugGUI::Instance().AddLog("\n");

	// アニメーション時間の35％から100％の間、攻撃判定有効
	if (m_animeTime >= 0.35f && m_animeTime <= 1.0f)
	{
		m_bossEnemy->UpdateAttackCollision(5.0f, 1.0f, 1, 0.3f);
	}

	// Rが終わったら必ずIdleで1秒休む
	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto next = std::make_shared<BossEnemyState_Idle>(1.0f);
		m_bossEnemy->ChangeState(next);
		return;
	}

	if (m_time < 0.2f)
	{
		const float dashSpeed = 0.7f;
		m_bossEnemy->SetIsMoving(m_attackDirection * dashSpeed);
	}
	else
	{
		m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
	}
}

void BossEnemyState_Attack_R::StateEnd()
{
	m_bossEnemy->SetInvincible(false);
}