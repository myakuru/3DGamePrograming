#include "EnemyState_Idle.h"
#include"../../../Player/Player.h"
#include"../EnemyState_Run/EnemyState_Run.h"

void EnemyState_Idle::StateStart()
{
	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Idle");
	m_enemy->GetAnimator()->SetAnimation(anime);

	m_enemy->SetAnimeSpeed(60.0f);

}

void EnemyState_Idle::StateUpdate()
{

	// 移動量リセット
	m_enemy->SetIsMoving(Math::Vector3::Zero);

	auto player = m_enemy->GetPlayerWeakPtr();
	Math::Vector3 playerPos = player.lock()->GetPos();
	Math::Vector3 enemyPos = m_enemy->GetPos();

	// 距離計算
	m_distance = (playerPos - enemyPos).Length();

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
