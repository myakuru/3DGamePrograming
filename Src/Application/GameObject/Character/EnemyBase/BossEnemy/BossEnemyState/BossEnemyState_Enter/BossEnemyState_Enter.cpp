#include "BossEnemyState_Enter.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"Application/Scene/SceneManager.h"
#include"Application/GameObject/Effect/EffekseerEffect/BossEnemyEnterEffect/BossEnemyEnterEffect.h"

void BossEnemyState_Enter::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Anim_Enter");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	m_effectPlayed = false;
}

void BossEnemyState_Enter::StateUpdate()
{
	// アニメーションの再生時間を取得
	m_animeTime = m_bossEnemy->GetAnimator()->GetPlayProgress();

	SceneManager::Instance().GetObjectWeakPtr(m_enterEffect);

	if (!m_effectPlayed)
	{
		if (auto effect = m_enterEffect.lock())
		{
			effect->SetPlayEffect(true);
			m_effectPlayed = true;
		}
	}

	if (m_bossEnemy->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<BossEnemyState_Idle>();
		m_bossEnemy->ChangeState(state);
		return;
	}

	m_bossEnemy->SetIsMoving(Math::Vector3::Zero);
}

void BossEnemyState_Enter::StateEnd()
{
	if (auto effect = m_enterEffect.lock())
	{
		effect->SetPlayEffect(false);
	}
}

void BossEnemyState_Enter::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_Enter));
	const auto& p = static_cast<const BossEnemyState_Enter&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
}

void BossEnemyState_Enter::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
}

void BossEnemyState_Enter::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("BossEnemyState_Enter")) return;
	const auto& stateNode = js["BossEnemyState_Enter"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
	}
}

void BossEnemyState_Enter::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("BossEnemyState_Enter")) js["BossEnemyState_Enter"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_Enter"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
}
