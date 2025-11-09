#include "EnemyState_Dath.h"

void EnemyState_Death::StateStart()
{
	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Death");
	m_enemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	m_enemy->SetAnimeSpeed(60.0f);

	m_enemy->SetInvincible(true); // 無敵状態にする

	m_time = 0.0f;
}

void EnemyState_Death::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_enemy->SetIsMoving(Math::Vector3::Zero);

	// 死亡アニメが終わったらディゾルブ開始
	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		float d = m_enemy->GetDissolve();
		d += 2.0f * deltaTime;

		if (d >= 1.0f)
		{
			m_enemy->SetDissolve(1.0f);
			m_enemy->SetExpired(true);
		}
		else
		{
			m_enemy->SetDissolve(d);
		}
	}
}

void EnemyState_Death::StateEnd()
{
	m_enemy->SetInvincible(false); // 無敵状態にする
}
