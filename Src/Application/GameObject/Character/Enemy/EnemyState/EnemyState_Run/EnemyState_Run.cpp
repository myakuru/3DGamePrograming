#include "EnemyState_Run.h"
#include"../../../Player/Player.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Attack/EnemyState_Attack.h"
#include"../EnemyState_Walk_Right/EnemyState_Walk_Right.h"

void EnemyState_Run::StateStart()
{
	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Run");
	m_enemy->GetAnimator()->SetAnimation(anime);
	m_enemy->SetAnimeSpeed(60.0f);
}

void EnemyState_Run::StateUpdate()
{
	if (auto player = m_enemy->GetPlayerWeakPtr().lock(); player)
	{
		m_playerPos = player->GetPos();
		m_enemyPos = m_enemy->GetPos();
	}

	// 距離計算
	m_distance = (m_playerPos - m_enemyPos).Length();

	if (m_distance >= 6.0f)
	{
		// Idleステートに移行
		auto spIdleState = std::make_shared<EnemyState_Idle>();
		m_enemy->ChangeState(spIdleState);
		return;
	}
	else if (m_distance >= 3.0f)
	{
		// 追いかける
		Math::Vector3 dir = m_playerPos - m_enemyPos;
		dir.y = 0.0f;
		if (dir != Math::Vector3::Zero) dir.Normalize();

		// LookRotationで正しい向きに
		Math::Quaternion rot = Math::Quaternion::LookRotation(dir, Math::Vector3::Up);
		m_enemy->SetRotation(rot);

		m_enemy->SetIsMoving(dir);
	}
	else
	{
		//Attackステートに移行
		auto attack = std::make_shared<EnemyState_Walk_Right>();
		m_enemy->ChangeState(attack);
		return;
	}
}

void EnemyState_Run::StateEnd()
{
}
