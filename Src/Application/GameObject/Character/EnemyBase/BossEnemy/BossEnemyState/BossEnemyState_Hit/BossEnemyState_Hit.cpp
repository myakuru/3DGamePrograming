#include "BossEnemyState_Hit.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../BossEnemyState_Attack_L/BossEnemyState_Attack_L.h"
#include"../BossEnemyAI.h"

void BossEnemyState_Hit::StateStart(BossEnemy* _owner)
{
	auto anime = _owner->GetAnimeModel()->GetAnimation("Hit");
	_owner->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	BossEnemyStateBase::StateStart(_owner);
	// アニメーション速度を変更
	_owner->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 累積ヒット回数は Enemy 本体で管理
	_owner->IncrementHitCount();

	m_hitCount = 10;

	// 10回以上で無敵
	if (_owner->GetHitCount() >= m_hitCount)
	{
		_owner->SetInvincible(true);
		_owner->ResetHitCount();
	}
}

void BossEnemyState_Hit::StateUpdate(BossEnemy* _owner)
{
	// 無敵中
	if (_owner->GetInvincible())
	{
		auto next = _owner->GetBossEnemyAI()->DecideNext(_owner);
		_owner->ChangeState(next);
		return;
	}

	// アニメーション終了
	if (_owner->GetAnimator()->IsAnimationEnd())
	{
		auto next = _owner->GetBossEnemyAI()->DecideNext(_owner);
		_owner->ChangeState(next);
		return;
	}

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		_owner->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
	}
	else
	{
		_owner->SetIsMoving(Math::Vector3::Zero);
	}
}

void BossEnemyState_Hit::StateEnd(BossEnemy* _owner)
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
