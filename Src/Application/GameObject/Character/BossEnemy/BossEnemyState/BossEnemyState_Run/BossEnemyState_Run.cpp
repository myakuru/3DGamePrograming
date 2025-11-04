#include "BossEnemyState_Run.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../BossEnemyState_Attack_L/BossEnemyState_Attack_L.h"
#include"../../../Player/Player.h"
#include"../BossEnemyAI.h" 

void BossEnemyState_Run::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Walk");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, 0.25f, true);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(60.0f);

	// 行動記録
	m_bossEnemy->SetLastAction(BossEnemy::ActionType::Run);
}

void BossEnemyState_Run::StateUpdate()
{
	if (auto player = m_bossEnemy->GetPlayerWeakPtr().lock(); player)
	{
		m_playerPos = player->GetPos();
		m_enemyPos = m_bossEnemy->GetPos();
	}

	// 距離計算
	m_distance = (m_playerPos - m_enemyPos).Length();

	// 10m未満になったらAIに委譲
	if (m_distance < 10.0f)
	{
		auto next = BossEnemyAI::DecideNext(m_bossEnemy);

		// 自分と同じ Run への再遷移は行わない（毎フレームアニメを張り直すのを防ぐ）
		if (!std::dynamic_pointer_cast<BossEnemyState_Run>(next))
		{
			m_bossEnemy->ChangeState(next);
			return;
		}
	}

	// 追いかける（状態維持時のみ適用）
	Math::Vector3 dir = m_playerPos - m_enemyPos;
	dir.y = 0.0f;

	// ゼロベクトル対策
	if (dir.LengthSquared() > 1e-8f)
	{
		dir.Normalize();

		// LookRotationで正しい向きに（ゼロベクトルは渡さない）
		Math::Quaternion rot = Math::Quaternion::LookRotation(dir, Math::Vector3::Up);
		m_bossEnemy->SetRotation(rot);
	}

	m_bossEnemy->SetIsMoving(dir);
}

void BossEnemyState_Run::StateEnd()
{
}
