#include "BossEnemyState_Enter.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/BossEnemyEnterEffect/BossEnemyEnterEffect.h"

void BossEnemyState_Enter::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Anim_Enter");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(60.0f);

	m_effectPlayed = false;
}

void BossEnemyState_Enter::StateUpdate()
{
	// アニメーションの再生時間を取得
	m_animeTime = m_bossEnemy->GetAnimator()->GetPlayProgress();

	SceneManager::Instance().GetObjectWeakPtr(m_enterEffect);

	if (m_animeTime >= 0.2f && !m_effectPlayed)
	{
		if (auto effect = m_enterEffect.lock(); effect)
		{
			effect->SetPlayEffect(true);
			m_effectPlayed = true;
		}
	}

	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<BossEnemyState_Idle>();
		m_bossEnemy->ChangeState(state);
		return;
	}

	m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
}

void BossEnemyState_Enter::StateEnd()
{
	if (auto effect = m_enterEffect.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}
}
