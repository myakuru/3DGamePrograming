#include "BossEnemyState_WaterFall.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/BossWaterFallAttack/BossWaterFallAttack.h"

void BossEnemyState_WaterFall::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Attack_WaterFall");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(60.0f);
	// 当たり判定リセット
	m_bossEnemy->ResetAttackCollision();
	m_bossEnemy->SetStateChange(true);

	m_bossEnemy->SetInvincible(true);

	// 追加: 行動CDと直前行動
	m_bossEnemy->SetWaterCooldown(2.0f);
	m_bossEnemy->SetLastAction(BossEnemy::ActionType::WaterFall);

	SceneManager::Instance().GetObjectWeakPtr(m_waterFallAttack);
	
}

void BossEnemyState_WaterFall::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();
	m_time += deltaTime;

	// アニメーション再生時間を取得
	m_animeTime = m_bossEnemy->GetAnimator()->GetPlayProgress();

	if (m_animeTime >= 0.8f)
	{
		m_bossEnemy->UpdateAttackCollision(6.0f, 3.0f, 5, 1.0f);

		if (auto effect = m_waterFallAttack.lock(); effect)
		{
			// エフェクトの初期化
			effect->SetPlayEffect(true);
		}
	}


	if (m_time < 0.3f)
	{
		const float dashSpeed = 0.0f;
		m_bossEnemy->SetIsMoving(m_attackDirection * dashSpeed);
	}
	else
	{
		m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
	}

	// アニメーションが終了したら必ずIdleへ遷移し、1秒待機
	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto next = std::make_shared<BossEnemyState_Idle>(1.0f);
		m_bossEnemy->ChangeState(next);
		return;
	}
}

void BossEnemyState_WaterFall::StateEnd()
{
	m_bossEnemy->SetInvincible(false);
	if (auto effect = m_waterFallAttack.lock(); effect)
	{
		// エフェクトの初期化
		effect->SetPlayEffect(false);
		effect->StopEffect();
	}

}
