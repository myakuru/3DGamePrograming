#include "BossEnemyState_ChrgeLoop.h"
#include"../BossEnemyState_ChargeEnd/BossEnemyState_ChargeEnd.h"
#include"Application/main.h"

void BossEnemyState_ChrgeLoop::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("ChargeLoop");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, true);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(m_stateParameter.animationSpeed);
}

void BossEnemyState_ChrgeLoop::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// アニメーション終了で次のステートへ
	if (m_time >= m_chargeTime)
	{
		auto nextState = std::make_shared<BossEnemyState_ChargeEnd>();
		m_bossEnemy->ChangeState(nextState);
		return;
	}

	m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
}

void BossEnemyState_ChrgeLoop::StateEnd()
{
}

void BossEnemyState_ChrgeLoop::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_ChrgeLoop));
	const auto& p = static_cast<const BossEnemyState_ChrgeLoop&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_chargeTime = p.m_chargeTime;
}

void BossEnemyState_ChrgeLoop::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
	ImGui::DragFloat(U8("チャージ時間"), &m_chargeTime);
}

void BossEnemyState_ChrgeLoop::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("BossEnemyState_ChrgeLoop")) return;
	const auto& stateNode = js["BossEnemyState_ChrgeLoop"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
		if (enemyNode.contains("chargeTime")) m_chargeTime = enemyNode["chargeTime"].get<float>();
	}
}

void BossEnemyState_ChrgeLoop::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("BossEnemyState_ChrgeLoop")) js["BossEnemyState_ChrgeLoop"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_ChrgeLoop"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["BossEnemy"]["chargeTime"] = m_chargeTime;
}
