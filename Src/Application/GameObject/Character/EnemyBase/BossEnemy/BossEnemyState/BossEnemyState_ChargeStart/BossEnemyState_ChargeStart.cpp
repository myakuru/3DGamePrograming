#include "BossEnemyState_ChargeStart.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_ChrgeLoop/BossEnemyState_ChargeLoop.h"

void BossEnemyState_ChargeStart::StateStart(BossEnemy* _owner)
{
	auto anime = _owner->GetAnimeModel()->GetAnimation("ChargeStart");
	_owner->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	BossEnemyStateBase::StateStart(_owner);

	// アニメーション速度を変更
	_owner->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 当たり判定リセット
	_owner->ResetAttackCollision();
}

void BossEnemyState_ChargeStart::StateUpdate(BossEnemy* _owner)
{
	// アニメーション終了で次のステートへ
	if (_owner->GetAnimator()->IsAnimationEnd())
	{
		auto nextState = std::make_shared<BossEnemyState_ChargeLoop>();
		_owner->ChangeState(nextState);
		return;
	}

	_owner->SetIsMoving(Math::Vector3::Zero);
}

void BossEnemyState_ChargeStart::StateEnd(BossEnemy* _owner)
{
}

void BossEnemyState_ChargeStart::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_ChargeStart));
	const auto& p = static_cast<const BossEnemyState_ChargeStart&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
}

void BossEnemyState_ChargeStart::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
}

void BossEnemyState_ChargeStart::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("BossEnemyState_ChargeStart")) return;
	const auto& stateNode = js["BossEnemyState_ChargeStart"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
	}
}

void BossEnemyState_ChargeStart::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("BossEnemyState_ChargeStart")) js["BossEnemyState_ChargeStart"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_ChargeStart"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
}
