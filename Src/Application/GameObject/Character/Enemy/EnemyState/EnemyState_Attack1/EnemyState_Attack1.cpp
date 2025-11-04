#include "EnemyState_Attack1.h"
#include"../EnemyState_Attack2/EnemyState_Attack2.h"
#include"../EnemyState_Run/EnemyState_Run.h"

#include"../../../Player/Player.h"

#include"../../../../../main.h"

void EnemyState_Attack1::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Attack1");
	m_enemy->GetAnimator()->SetAnimation(anime, 0.25f, false);

	m_time = 0.0f;

	// 当たり判定リセット
	m_enemy->ResetAttackCollision();

}

void EnemyState_Attack1::StateUpdate()
{
	// 攻撃中の移動方向で回転を更新
	if (m_enemy->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_enemy->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_enemy->UpdateQuaternionDirect(moveDir);
	}


	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	m_animeTime = m_enemy->GetAnimator()->GetPlayProgress();

	// アニメーション時間の35％から100％の間、攻撃判定有効
	if (m_animeTime >= 0.35f && m_animeTime <= 1.0f)
	{
		m_enemy->UpdateAttackCollision(2.0f, 1.0f, 1, 0.3f);
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

	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		auto attack = std::make_shared<EnemyState_Attack2>();
		m_enemy->ChangeState(attack);
		return;
	}

	if (m_time < 0.2f)
	{
		const float dashSpeed = 0.7f;
		m_enemy->SetIsMoving(m_attackDirection * dashSpeed);
	}
	else
	{
		m_enemy->SetIsMoving(Math::Vector3::Zero);
	}
}

void EnemyState_Attack1::StateEnd()
{
	m_enemy->SetInvincible(false);
}
