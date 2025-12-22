#include "EnemyState_Run.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Attack/EnemyState_Attack.h"
#include"../EnemyState_Walk_Right/EnemyState_Walk_Right.h"

#include"Application/main.h"
#include"Application/GameObject/Character/Player/Player.h"

void EnemyState_Run::StateStart(RedEnemy* _owner)
{
	EnemyStateBase::StateStart(_owner);

	auto anime = _owner->GetAnimeModel()->GetAnimation("Run");
	_owner->GetAnimator()->SetAnimation(anime);
	_owner->SetAnimeSpeed(m_stateParameter.animationSpeed);
}

void EnemyState_Run::StateUpdate(RedEnemy* _owner)
{
	for (const auto& player : m_player)
	{
		if (auto p = player.lock())
		{
			m_playerPos = p->GetPos();
			m_enemyPos = _owner->GetPos();
			break;
		}
	}

	// 距離計算
	m_distance = (m_playerPos - m_enemyPos).Length();

	if (m_distance >= m_stateParameter.distanceThreshold)
	{
		// Idleステートに移行
		auto spIdleState = std::make_shared<EnemyState_Idle>();
		_owner->ChangeState(spIdleState);
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
		_owner->SetRotation(rot);

		_owner->SetIsMoving(dir);
	}
	else
	{
		//Attackステートに移行
		auto attack = std::make_shared<EnemyState_Walk_Right>();
		_owner->ChangeState(attack);
		return;
	}
}

void EnemyState_Run::StateEnd(RedEnemy* _owner)
{
}

void EnemyState_Run::ApplyFromConfig(const EnemyStateBase& other)
{
	assert(typeid(other) == typeid(EnemyState_Run));
	const auto& p = static_cast<const EnemyState_Run&>(other);
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.distanceThreshold = p.m_stateParameter.distanceThreshold;
}

void EnemyState_Run::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::DragFloat(U8("距離閾値"), &m_stateParameter.distanceThreshold);
}

void EnemyState_Run::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("EnemyState_Run")) return;
	const auto& stateNode = js["EnemyState_Run"];
	if (stateNode.contains("AetheriusEnemy"))
	{
		const auto& enemyNode = stateNode["AetheriusEnemy"];
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
		if (enemyNode.contains("distanceThreshold")) m_stateParameter.distanceThreshold = enemyNode["distanceThreshold"].get<float>();
	}
}

void EnemyState_Run::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("EnemyState_Run")) js["EnemyState_Run"] = nlohmann::json::object();
	auto& stateNode = js["EnemyState_Run"];
	stateNode["AetheriusEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["AetheriusEnemy"]["distanceThreshold"] = m_stateParameter.distanceThreshold;
}
