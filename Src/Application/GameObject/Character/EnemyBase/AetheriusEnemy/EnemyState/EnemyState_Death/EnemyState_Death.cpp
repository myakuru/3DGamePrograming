#include "EnemyState_Death.h"

void EnemyState_Death::StateStart(RedEnemy* _owner)
{
	EnemyStateBase::StateStart(_owner);

	auto anime = _owner->GetAnimeModel()->GetAnimation("Death");
	_owner->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	_owner->SetAnimeSpeed(m_stateParameter.animationSpeed);

	_owner->SetInvincible(true); // 無敵状態にする

	m_time = 0.0f;
}

void EnemyState_Death::StateUpdate(RedEnemy* _owner)
{
	float deltaTime = Application::Instance().GetDeltaTime();

	_owner->SetIsMoving(Math::Vector3::Zero);

	// 死亡アニメが終わったらディゾルブ開始
	if (_owner->GetAnimator()->IsAnimationEnd())
	{
		float d = _owner->GetDissolve();
		d += 2.0f * deltaTime;

		if (d >= 1.0f)
		{
			_owner->SetDissolve(1.0f);
			_owner->SetExpired(true);
		}
		else
		{
			_owner->SetDissolve(d);
		}
	}
}

void EnemyState_Death::StateEnd(RedEnemy* _owner)
{
	_owner->SetInvincible(false); // 無敵状態にする
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
