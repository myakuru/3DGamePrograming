#include "BossEnemyState_Idle.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"../BossEnemyState_Run/BossEnemyState_Run.h"
#include"../BossEnemyAI.h"
#include"Application/main.h"

void BossEnemyState_Idle::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Idle");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, true);
	BossEnemyStateBase::StateStart();
	m_bossEnemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	m_bossEnemy->SetLastAction(BossEnemy::ActionType::Idle);
}

void BossEnemyState_Idle::StateUpdate()
{
	const float deltaTime = Application::Instance().GetDeltaTime();
	m_time += deltaTime;

	// 停止
	m_bossEnemy->SetIsMoving(Math::Vector3::Zero);

	// 最低待機時間まではそのまま
	if (m_time < m_minWaitSec) return;

	// 待機後はAIに委譲
	auto next = BossEnemyAI::DecideNext(m_bossEnemy);
	m_bossEnemy->ChangeState(next);
}

void BossEnemyState_Idle::StateEnd()
{
}

void BossEnemyState_Idle::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_Idle));
	const auto& p = static_cast<const BossEnemyState_Idle&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
}

void BossEnemyState_Idle::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
}

void BossEnemyState_Idle::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("BossEnemyState_Idle")) return;
	const auto& stateNode = js["BossEnemyState_Idle"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
	}
}

void BossEnemyState_Idle::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("BossEnemyState_Idle")) js["BossEnemyState_Idle"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_Idle"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
}
