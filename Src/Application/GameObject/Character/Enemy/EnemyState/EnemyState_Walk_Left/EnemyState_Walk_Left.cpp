#include "EnemyState_Walk_Left.h"
#include"../../../Player/Player.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Attack/EnemyState_Attack.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include"../../../../../main.h"

void EnemyState_Walk_Left::StateStart()
{
	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Walk_Left");
	m_enemy->GetAnimator()->SetAnimation(anime);
	m_enemy->SetAnimeSpeed(60.0f);

	m_time = 0.0f;
}

void EnemyState_Walk_Left::StateUpdate()
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
	Math::Vector3 right = Math::Vector3::TransformNormal
	(
		Math::Vector3::Right,
		Math::Matrix::CreateFromQuaternion(m_enemy->GetRotationQuaternion())
	);

	right.Normalize();

	// 右に少しずつ移動
	m_enemy->SetIsMoving(right * 0.2f);

	if (m_time >= 2.0f)
	{
		//Attackステートに移行
		auto attack = std::make_shared<EnemyState_Attack>();
		m_enemy->ChangeState(attack);
		return;
	}
}

void EnemyState_Walk_Left::StateEnd()
{
}
