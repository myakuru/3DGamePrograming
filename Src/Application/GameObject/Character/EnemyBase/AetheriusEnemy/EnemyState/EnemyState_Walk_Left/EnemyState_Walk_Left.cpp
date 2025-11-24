#include "EnemyState_Walk_Left.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Attack/EnemyState_Attack.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include"Application/main.h"
#include"Application/GameObject/Character/Player/Player.h"

void EnemyState_Walk_Left::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Walk_Left");
	m_enemy->GetAnimator()->SetAnimation(anime);
	m_enemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	m_stateParameter.changeStateTime = 2.0f;
	m_stateParameter.dashSpeed = 0.2f;
}

void EnemyState_Walk_Left::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// 距離が６以上離れたら追いかける
	{
		for (const auto& player : m_player)
		{
			if (auto p = player.lock())
			{
				m_playerPos = p->GetPos();
				m_enemyPos = m_enemy->GetPos();
				break;
			}
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
	m_enemy->SetIsMoving(right * m_stateParameter.dashSpeed);

	if (m_time >= m_stateParameter.changeStateTime)
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

void EnemyState_Walk_Left::ApplyFromConfig(const EnemyStateBase& other)
{
	assert(typeid(other) == typeid(EnemyState_Walk_Left));
	const auto& p = static_cast<const EnemyState_Walk_Left&>(other);
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.changeStateTime = p.m_stateParameter.changeStateTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
}

void EnemyState_Walk_Left::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::DragFloat(U8("状態遷移までの時間"), &m_stateParameter.changeStateTime);
	ImGui::DragFloat(U8("横移動速度"), &m_stateParameter.dashSpeed);
}

void EnemyState_Walk_Left::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("EnemyState_Walk_Left")) return;
	const auto& stateNode = js["EnemyState_Walk_Left"];
	if (stateNode.contains("AetheriusEnemy"))
	{
		const auto& enemyNode = stateNode["AetheriusEnemy"];
		if (enemyNode.contains("animationSpeed"))
		{
			m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
			m_stateParameter.changeStateTime = enemyNode["changeStateTime"].get<float>();
			m_stateParameter.dashSpeed = enemyNode["dashSpeed"].get<float>();
		}
	}
}

void EnemyState_Walk_Left::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("EnemyState_Walk_Left")) js["EnemyState_Walk_Left"] = nlohmann::json::object();
	auto& stateNode = js["EnemyState_Walk_Left"];
	stateNode["AetheriusEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["AetheriusEnemy"]["changeStateTime"] = m_stateParameter.changeStateTime;
	stateNode["AetheriusEnemy"]["dashSpeed"] = m_stateParameter.dashSpeed;
}
