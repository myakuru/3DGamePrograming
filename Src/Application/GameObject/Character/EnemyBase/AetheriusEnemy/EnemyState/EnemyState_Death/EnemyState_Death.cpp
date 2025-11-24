#include "EnemyState_Death.h"

void EnemyState_Death::StateStart()
{
	EnemyStateBase::StateStart();

	auto anime = m_enemy->GetAnimeModel()->GetAnimation("Death");
	m_enemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	m_enemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	m_enemy->SetInvincible(true); // 無敵状態にする

	m_time = 0.0f;
}

void EnemyState_Death::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_enemy->SetIsMoving(Math::Vector3::Zero);

	// 死亡アニメが終わったらディゾルブ開始
	if (m_enemy->GetAnimator()->IsAnimationEnd())
	{
		float d = m_enemy->GetDissolve();
		d += 2.0f * deltaTime;

		if (d >= 1.0f)
		{
			m_enemy->SetDissolve(1.0f);
			m_enemy->SetExpired(true);
		}
		else
		{
			m_enemy->SetDissolve(d);
		}
	}
}

void EnemyState_Death::StateEnd()
{
	m_enemy->SetInvincible(false); // 無敵状態にする
}

void EnemyState_Death::ApplyFromConfig(const EnemyStateBase& other)
{
	assert(typeid(other) == typeid(EnemyState_Death));
	const auto& p = static_cast<const EnemyState_Death&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
}

void EnemyState_Death::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
}

void EnemyState_Death::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("EnemyState_Death")) return;
	const auto& stateNode = js["EnemyState_Death"];
	if (stateNode.contains("AetheriusEnemy"))
	{
		const auto& enemyNode = stateNode["AetheriusEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
	}
}

void EnemyState_Death::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("EnemyState_Death")) js["EnemyState_Death"] = nlohmann::json::object();
	auto& stateNode = js["EnemyState_Death"];

	stateNode["AetheriusEnemy"]["blendTime"] = m_stateParameter.blendTime;
}
