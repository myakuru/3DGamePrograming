#include "EnemyState_Idle.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include "Application/Scene/SceneManager.h"
#include "Application/GameObject/Character/Player/Player.h"

void EnemyState_Idle::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Idle");
	m_enemy->GetAnimator()->SetAnimation(anime);

	m_enemy->SetAnimeSpeed(60.0f);

}

void EnemyState_Idle::StateUpdate()
{

	// 移動量リセット
	m_enemy->SetIsMoving(Math::Vector3::Zero);

	for (const auto& player : m_player)
	{
		if (auto p = player.lock())
		{
			m_playerPos = p->GetPos();
			m_enemyPos = m_enemy->GetPos();
		}
	}

	// 距離計算
	m_distance = (m_playerPos - m_enemyPos).Length();

	if (m_distance < 6.0f)
	{
		// Runステートに移行
		auto spRunState = std::make_shared<EnemyState_Run>();
		m_enemy->ChangeState(spRunState);
		return;
	}
}

void EnemyState_Idle::StateEnd()
{
}
