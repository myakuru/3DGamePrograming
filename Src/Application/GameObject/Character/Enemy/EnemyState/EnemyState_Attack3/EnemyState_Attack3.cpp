#include "EnemyState_Attack3.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"

void EnemyState_Attack3::StateStart()
{

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Attack2");
	m_enemy->GetAnimator()->AnimationBlend(anime, 10.0f, false);
	m_enemy->AnimeSetFlg() = true;

	m_enemy->m_onceEffect = false;
}

void EnemyState_Attack3::StateUpdate()
{
	m_enemy->SetAnimeSpeed(60.0f);

	m_enemy->UpdateAttack();

	// 移動量リセット
	m_enemy->SetIsMoving(Math::Vector3::Zero);

	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		auto idle = std::make_shared<EnemyState_Idle>();
		m_enemy->ChangeState(idle);
		return;
	}
}

void EnemyState_Attack3::StateEnd()
{
}
