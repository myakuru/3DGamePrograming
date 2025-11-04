#include "EnemyState_Attack.h"
#include"../EnemyState_Attack1/EnemyState_Attack1.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../../main.h"
#include"../../../../Effect/EffekseerEffect/EnemyShineBlue/EnemyShineBlue.h"
#include"../../../Player/Player.h"

void EnemyState_Attack::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Attack");
	m_enemy->GetAnimator()->SetAnimation(anime, 0.25f, false);

	for (const auto& effects : m_shineEffectBlues)
	{
		if (auto effect = effects.lock(); effect)
		{
			effect->SetPlayEffect(true);
			break;
		}
	}

	// 当たり判定リセット
	m_enemy->ResetAttackCollision();
}

void EnemyState_Attack::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// アニメーション再生時間を取得
	m_animeTime = m_enemy->GetAnimator()->GetPlayProgress();

	KdDebugGUI::Instance().AddLog("EnemyAttackAnimeTime:%f", m_animeTime);
	KdDebugGUI::Instance().AddLog("\n");

	// アニメーション時間の35％から100％の間、攻撃判定有効
	if (m_animeTime >= 0.35f && m_animeTime <= 1.0f)
	{
		m_enemy->UpdateAttackCollision(10.0f, 1.0f, 1, 0.3f);
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
		auto attack1 = std::make_shared<EnemyState_Attack1>();
		m_enemy->ChangeState(attack1);
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

void EnemyState_Attack::StateEnd()
{
	for (const auto& effects : m_shineEffectBlues)
	{
		if (auto effect = effects.lock(); effect)
		{
			effect->SetPlayEffect(false);
			break;
		}
	}
}
