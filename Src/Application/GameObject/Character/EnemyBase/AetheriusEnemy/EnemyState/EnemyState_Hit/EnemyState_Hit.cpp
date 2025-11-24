#include "EnemyState_Hit.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Attack/EnemyState_Attack.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"Application/Scene/SceneManager.h"

void EnemyState_Hit::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Hit");
	m_enemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	m_enemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 累積ヒット回数は Enemy 本体で管理
	m_enemy->IncrementHitCount();

	m_stateParameter.dashSpeedTime = 0.2f;
	m_stateParameter.dashSpeed = -0.5f;
}

void EnemyState_Hit::StateUpdate()
{
	if (m_enemy->GetInvincible())
	{
		// 無敵状態ならAttackへ
		auto spIdleState = std::make_shared<EnemyState_Attack>();
		m_enemy->ChangeState(spIdleState);
		return;
	}


	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		// Idleステートに移行
		auto spIdleState = std::make_shared<EnemyState_Idle>();
		m_enemy->ChangeState(spIdleState);
		return;
	}

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_enemy->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
	}
	else
	{
		m_enemy->SetIsMoving(Math::Vector3::Zero);
	}
}

void EnemyState_Hit::StateEnd()
{
	// 10回以上で無敵
	if (m_enemy->GetHitCount() >= 10)
	{
		m_enemy->SetInvincible(true);
		m_enemy->ResetHitCount();
	}
}

void EnemyState_Hit::ApplyFromConfig(const EnemyStateBase& other)
{
	assert(typeid(other) == typeid(EnemyState_Hit));
	const auto& p = static_cast<const EnemyState_Hit&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;
}

void EnemyState_Hit::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("ダッシュ移動速度"), &m_stateParameter.dashSpeed);
	ImGui::DragFloat(U8("ダッシュ移動速度時間"), &m_stateParameter.dashSpeedTime);
}

void EnemyState_Hit::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("EnemyState_Hit")) return;
	const auto& stateNode = js["EnemyState_Hit"];
	if (stateNode.contains("AetheriusEnemy"))
	{
		const auto& enemyNode = stateNode["AetheriusEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("dashSpeed")) m_stateParameter.dashSpeed = enemyNode["dashSpeed"].get<float>();
		if (enemyNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = enemyNode["dashSpeedTime"].get<float>();
	}
}

void EnemyState_Hit::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("EnemyState_Hit")) js["EnemyState_Hit"] = nlohmann::json::object();
	auto& stateNode = js["EnemyState_Hit"];

	stateNode["AetheriusEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["AetheriusEnemy"]["dashSpeed"] = m_stateParameter.dashSpeed;
	stateNode["AetheriusEnemy"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;
}
