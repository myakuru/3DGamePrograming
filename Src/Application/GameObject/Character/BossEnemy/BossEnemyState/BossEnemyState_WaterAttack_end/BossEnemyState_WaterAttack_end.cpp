#include "BossEnemyState_WaterAttack_end.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../BossEnemyAI.h"

void BossEnemyState_WaterAttack_end::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Dodge");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(60.0f);
	// 当たり判定リセット
	m_bossEnemy->ResetAttackCollision();
}

void BossEnemyState_WaterAttack_end::StateUpdate()
{
	// アニメーション再生時間を取得
	m_animeTime = m_bossEnemy->GetAnimator()->GetPlayProgress();

	// アニメーション時間の35％から100％の間、攻撃判定有効（必要なら残す）
	if (m_animeTime >= 0.35f && m_animeTime <= 1.0f)
	{
		m_bossEnemy->UpdateAttackCollision(10.0f, 1.0f, 1, 0.3f);
	}

	// アニメーションが終了したら必ずIdleへ遷移し、1秒待機
	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto next = std::make_shared<BossEnemyState_Idle>(1.0f);
		m_bossEnemy->ChangeState(next);
		return;
	}

	m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
}

void BossEnemyState_WaterAttack_end::StateEnd()
{
	m_bossEnemy->SetStateChange(false);
}