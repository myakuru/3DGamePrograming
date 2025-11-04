#include "EnemState_Attack3.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include"../../../Player/Player.h"

void EnemState_Attack3::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Attack3");
	m_enemy->GetAnimator()->SetAnimation(anime, 0.25f, false);
	m_enemy->SetAnimeSpeed(60.0f);

	// 当たり判定リセット
	m_enemy->ResetAttackCollision();
}

void EnemState_Attack3::StateUpdate()
{

	// アニメーションの再生時間を取得
	m_animeTime = m_enemy->GetAnimator()->GetPlayProgress();

	// アニメーション時間の35％から100％の間、攻撃判定有効
	if (m_animeTime >= 0.0f && m_animeTime <= 1.0f)
	{
		m_enemy->UpdateAttackCollision(2.0f, 0.0f, 1, 0.2f);
	}

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
	
	// 移動量リセット
	m_enemy->SetIsMoving(Math::Vector3::Zero);
	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		auto attack = std::make_shared<EnemyState_Idle>();
		m_enemy->ChangeState(attack);
		return;
	}
}

void EnemState_Attack3::StateEnd()
{
	m_enemy->SetInvincible(false);
}
