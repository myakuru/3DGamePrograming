#include "BossEnemyState_Dodge.h"
#include"../BossEnemyState_ChargeStart/BossEnemyState_ChargeStart.h"

void BossEnemyState_Dodge::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Dodge");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	BossEnemyStateBase::StateStart();

	m_bossEnemy->SetAnimeSpeed(60.0f);

	m_bossEnemy->SetInvincible(true);
	m_bossEnemy->SetStateChange(true);
}

void BossEnemyState_Dodge::StateUpdate()
{

	float deltaTime = Application::Instance().GetDeltaTime();
	m_time += deltaTime;

	if (m_time < 0.3f)
	{
		const float dashSpeed = -2.0f;
		m_bossEnemy->SetIsMoving(m_attackDirection * dashSpeed);
	}
	else
	{
		m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
	}

	// アニメーション終了で次のステートへ
	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto nextState = std::make_shared<BossEnemyState_ChargeStart>();
		m_bossEnemy->ChangeState(nextState);
		return;
	}
}

void BossEnemyState_Dodge::StateEnd()
{
}
