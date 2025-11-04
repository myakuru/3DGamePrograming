#include "BossEnemyState_ChrgeLoop.h"
#include"../BossEnemyState_ChargeEnd/BossEnemyState_ChargeEnd.h"

void BossEnemyState_ChrgeLoop::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("ChargeLoop");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, true);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(60.0f);
}

void BossEnemyState_ChrgeLoop::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// アニメーション終了で次のステートへ
	if (m_time >= 10.0f)
	{
		auto nextState = std::make_shared<BossEnemyState_ChargeEnd>();
		m_bossEnemy->ChangeState(nextState);
		return;
	}

	m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
}

void BossEnemyState_ChrgeLoop::StateEnd()
{
}
