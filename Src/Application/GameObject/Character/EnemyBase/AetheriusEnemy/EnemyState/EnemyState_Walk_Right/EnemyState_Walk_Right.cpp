#include "EnemyState_Walk_Right.h"
#include"../EnemyState_Idle/EnemyState_Idle.h"
#include"../EnemyState_Walk_Left/EnemyState_Walk_Left.h"
#include"../EnemyState_Run/EnemyState_Run.h"
#include"Application/main.h"
#include"Application/GameObject/Character/Player/Player.h"

void EnemyState_Walk_Right::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Walk_Right");
	m_enemy->GetAnimator()->SetAnimation(anime);
	m_enemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	m_stateParameter.changeStateTime = 2.0f;
	m_stateParameter.dashSpeed = 0.2f;
}

void EnemyState_Walk_Right::StateUpdate()
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
	Math::Vector3 left = Math::Vector3::TransformNormal
	(
		Math::Vector3::Left, 
		Math::Matrix::CreateFromQuaternion(m_enemy->GetRotationQuaternion())
	);

	left.Normalize();

	// 右に少しずつ移動
	m_enemy->SetIsMoving(left * m_stateParameter.dashSpeed);

	if(m_time >= m_stateParameter.changeStateTime)
	{
		//Attackステートに移行
		auto state = std::make_shared<EnemyState_Walk_Left>();
		m_enemy->ChangeState(state);
		return;
	}
	
}

void EnemyState_Walk_Right::StateEnd()
{
}

void EnemyState_Walk_Right::ApplyFromConfig(const EnemyStateBase& other)
{
	assert(typeid(other) == typeid(EnemyState_Walk_Right));
	const auto& p = static_cast<const EnemyState_Walk_Right&>(other);
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.changeStateTime = p.m_stateParameter.changeStateTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
}

void EnemyState_Walk_Right::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::DragFloat(U8("状態遷移までの時間"), &m_stateParameter.changeStateTime);
	ImGui::DragFloat(U8("横移動速度"), &m_stateParameter.dashSpeed);
}

void EnemyState_Walk_Right::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("EnemyState_Walk_Right")) return;
	const auto& stateNode = js["EnemyState_Walk_Right"];
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

void EnemyState_Walk_Right::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("EnemyState_Walk_Right")) js["EnemyState_Walk_Right"] = nlohmann::json::object();
	auto& stateNode = js["EnemyState_Walk_Right"];
	stateNode["AetheriusEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["AetheriusEnemy"]["changeStateTime"] = m_stateParameter.changeStateTime;
	stateNode["AetheriusEnemy"]["dashSpeed"] = m_stateParameter.dashSpeed;
}
