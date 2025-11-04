#include"EnemyStateBase.h"
#include"../../Player/Player.h"

void EnemyStateBase::StateStart()
{
	// 敵の方向ベクトルを計算
	if (auto player = m_enemy->GetPlayerWeakPtr().lock(); player)
	{
		Math::Vector3 enemyPos = m_enemy->GetPos();
		Math::Vector3 playerPos = player->GetPos();
		m_attackDirection = playerPos - enemyPos;

		// Y方向の成分を無視
		m_attackDirection.y = 0.0f;

		// 方向ベクトルがゼロベクトルでない場合に正規化して向きを更新
		if (m_attackDirection != Math::Vector3::Zero)
		{
			m_attackDirection.Normalize();

			const float yaw = atan2(-m_attackDirection.x, -m_attackDirection.z);
			Math::Quaternion rot = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::Up, yaw);
			m_enemy->SetRotation(rot);
		}
	}

	m_time = 0.0f;
	m_hasHitPlayer = false;
	m_animeTime = 0.0f;
}