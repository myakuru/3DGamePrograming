#include "BossEnemyState_Hit.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../BossEnemyState_Attack_L/BossEnemyState_Attack_L.h"
#include"../BossEnemyAI.h"

void BossEnemyState_Hit::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Hit");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 累積ヒット回数は Enemy 本体で管理
	m_bossEnemy->IncrementHitCount();

	m_hitCount = 10;

	// 10回以上で無敵
	if (m_bossEnemy->GetHitCount() >= m_hitCount)
	{
		m_bossEnemy->SetInvincible(true);
		m_bossEnemy->ResetHitCount();
	}
}

void BossEnemyState_Hit::StateUpdate()
{
	// 無敵中
	if (m_bossEnemy->GetInvincible())
	{
		auto next = BossEnemyAI::DecideNext(m_bossEnemy);
		m_bossEnemy->ChangeState(next);
		return;
	}

	// アニメーション終了
	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto next = BossEnemyAI::DecideNext(m_bossEnemy);
		m_bossEnemy->ChangeState(next);
		return;
	}

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_bossEnemy->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
	}
	else
	{
		m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
	}
}

void BossEnemyState_Hit::StateEnd()
{
}

void BossEnemyState_Hit::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_Hit));
	const auto& p = static_cast<const BossEnemyState_Hit&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
	m_hitCount = p.m_hitCount;
}

void BossEnemyState_Hit::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
	ImGui::DragFloat(U8("ダッシュ移動速度"), &m_stateParameter.dashSpeed, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat(U8("ダッシュ移動速度時間"), &m_stateParameter.dashSpeedTime, 0.01f, 0.0f, 5.0f);
	ImGui::DragInt(U8("無敵になるヒット回数"), &m_hitCount, 1, 1, 100);
}

void BossEnemyState_Hit::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("BossEnemyState_Hit")) return;
	const auto& stateNode = js["BossEnemyState_Hit"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
		if (enemyNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = enemyNode["dashSpeedTime"].get<float>();
		if (enemyNode.contains("dashSpeed")) m_stateParameter.dashSpeed = enemyNode["dashSpeed"].get<float>();
		if (enemyNode.contains("hitCount")) m_hitCount = enemyNode["hitCount"].get<int>();
	}
}

void BossEnemyState_Hit::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("BossEnemyState_Hit")) js["BossEnemyState_Hit"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_Hit"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["BossEnemy"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;
	stateNode["BossEnemy"]["dashSpeed"] = m_stateParameter.dashSpeed;
	stateNode["BossEnemy"]["hitCount"] = m_hitCount;
}
