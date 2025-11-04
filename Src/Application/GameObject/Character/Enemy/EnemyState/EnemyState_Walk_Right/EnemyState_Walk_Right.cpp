#include "EnemyState_Walk_Right.h"
#include"../../../Player/Player.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Walk_Left/EnemyState_Walk_Left.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include"../../../../../main.h"

void EnemyState_Walk_Right::StateStart()
{
	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Walk_Right");
	m_enemy->GetAnimator()->SetAnimation(anime);
	m_enemy->SetAnimeSpeed(60.0f);

	m_time = 0.0f;
}

void EnemyState_Walk_Right::StateUpdate()
{

	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// 距離が６以上離れたら追いかける
	{
		if (auto player = m_enemy->GetPlayerWeakPtr().lock(); player)
		{
			m_playerPos = player->GetPos();
			m_enemyPos = m_enemy->GetPos();
		}

		m_distance = (m_playerPos - m_enemyPos).Length();

		if (m_distance >= 6.0f)
		{
			auto state = std::make_shared<EnemyState_Run>();
			m_enemy->ChangeState(state);
			return;
		}
	}


	// 自身の横方向ベクトル
	Math::Vector3 left = Math::Vector3::TransformNormal
	(
		Math::Vector3::Left, 
		Math::Matrix::CreateFromQuaternion(m_enemy->GetRotationQuaternion())
	);

	left.Normalize();

	// 右に少しずつ移動
	m_enemy->SetIsMoving(left * 0.2f);

	if(m_time >= 2.0f)
	{
		//Attackステートに移行
		auto state = std::make_shared<EnemyState_Walk_Left>();
		m_enemy->ChangeState(state);
		return;
	}
	
}

void EnemyState_Walk_Right::StateEnd()
{
}
