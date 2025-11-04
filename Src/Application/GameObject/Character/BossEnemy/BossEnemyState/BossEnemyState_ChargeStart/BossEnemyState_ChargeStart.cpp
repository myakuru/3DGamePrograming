#include "BossEnemyState_ChargeStart.h"
#include"../BossEnemyState_ChrgeLoop/BossEnemyState_ChrgeLoop.h"

void BossEnemyState_ChargeStart::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("ChargeStart");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	BossEnemyStateBase::StateStart();
	// 当たり判定リセット
	m_bossEnemy->ResetAttackCollision();
}

void BossEnemyState_ChargeStart::StateUpdate()
{
	// アニメーション終了で次のステートへ
	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto nextState = std::make_shared<BossEnemyState_ChrgeLoop>();
		m_bossEnemy->ChangeState(nextState);
		return;
	}

	m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
}

void BossEnemyState_ChargeStart::StateEnd()
{
}
