#include "BossEnemyState_Dodge.h"
#include"../BossEnemyState_ChargeStart/BossEnemyState_ChargeStart.h"
#include"Application/main.h"

void BossEnemyState_Dodge::StateStart(BossEnemy* _owner)
{
	auto anime = _owner->GetAnimeModel()->GetAnimation("Dodge");
	_owner->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	BossEnemyStateBase::StateStart(_owner);

	_owner->SetAnimeSpeed(m_stateParameter.animationSpeed);

	_owner->SetInvincible(true);
}

void BossEnemyState_Dodge::StateUpdate(BossEnemy* _owner)
{

	float deltaTime = Application::Instance().GetDeltaTime();
	m_time += deltaTime;

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		_owner->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
	}
	else
	{
		_owner->SetIsMoving(Math::Vector3::Zero);
	}

	// アニメーション終了で次のステートへ
	if (_owner->GetAnimator()->IsAnimationEnd())
	{
		auto nextState = std::make_shared<BossEnemyState_ChargeStart>();
		_owner->ChangeState(nextState);
		return;
	}
}

void BossEnemyState_Dodge::StateEnd(BossEnemy* _owner)
{
}

void BossEnemyState_Dodge::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_Dodge));
	const auto& p = static_cast<const BossEnemyState_Dodge&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
}

void BossEnemyState_Dodge::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
	ImGui::DragFloat(U8("ダッシュ移動速度"), &m_stateParameter.dashSpeed, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat(U8("ダッシュ移動速度時間"), &m_stateParameter.dashSpeedTime, 0.01f, 0.0f, 5.0f);
}

void BossEnemyState_Dodge::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("BossEnemyState_Dodge")) return;
	const auto& stateNode = js["BossEnemyState_Dodge"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
		if (enemyNode.contains("dashSpeed")) m_stateParameter.dashSpeed = enemyNode["dashSpeed"].get<float>();
		if (enemyNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = enemyNode["dashSpeedTime"].get<float>();
	}
}

void BossEnemyState_Dodge::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("BossEnemyState_Dodge")) js["BossEnemyState_Dodge"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_Dodge"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["BossEnemy"]["dashSpeed"] = m_stateParameter.dashSpeed;
	stateNode["BossEnemy"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;
}
