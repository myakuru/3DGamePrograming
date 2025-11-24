#include "EnemyState_Idle.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include "Application/Scene/SceneManager.h"
#include "Application/GameObject/Character/Player/Player.h"

void EnemyState_Idle::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Idle");
	m_enemy->GetAnimator()->SetAnimation(anime);

	m_enemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

}

void EnemyState_Idle::StateUpdate()
{

	// 移動量リセット
	m_enemy->SetIsMoving(Math::Vector3::Zero);

	for (const auto& player : m_player)
	{
		if (auto p = player.lock())
		{
			m_playerPos = p->GetPos();
			m_enemyPos = m_enemy->GetPos();
		}
	}

	// 距離計算
	m_distance = (m_playerPos - m_enemyPos).Length();

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

void EnemyState_Idle::ApplyFromConfig(const EnemyStateBase& other)
{
	assert(typeid(other) == typeid(EnemyState_Idle));
	const auto& p = static_cast<const EnemyState_Idle&>(other);
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
}

void EnemyState_Idle::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
}

void EnemyState_Idle::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("EnemyState_Idle")) return;
	const auto& stateNode = js["EnemyState_Idle"];
	if (stateNode.contains("AetheriusEnemy"))
	{
		const auto& enemyNode = stateNode["AetheriusEnemy"];
		if (enemyNode.contains("animationSpeed"))
		{
			m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
		}
	}
}

void EnemyState_Idle::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("EnemyState_Idle")) js["EnemyState_Idle"] = nlohmann::json::object();
	auto& stateNode = js["EnemyState_Idle"];
	stateNode["AetheriusEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;

}
