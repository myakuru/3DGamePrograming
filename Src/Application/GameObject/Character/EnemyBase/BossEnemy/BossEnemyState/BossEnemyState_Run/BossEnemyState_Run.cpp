#include "BossEnemyState_Run.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"../BossEnemyState_Attack_L/BossEnemyState_Attack_L.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"../BossEnemyAI.h" 

void BossEnemyState_Run::StateStart()
{
	auto anime = m_bossEnemy->GetAnimeModel()->GetAnimation("Walk");
	m_bossEnemy->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, true);
	BossEnemyStateBase::StateStart();
	// アニメーション速度を変更
	m_bossEnemy->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 行動記録
	m_bossEnemy->SetLastAction(BossEnemy::ActionType::Run);
}

void BossEnemyState_Run::StateUpdate()
{
	const auto wp = m_bossEnemy->GetPlayerList();

	for (auto& weakPlayer : wp)
	{
		if (auto sp = weakPlayer.lock())
		{
			m_playerPos = sp->GetPos();
			m_enemyPos = m_bossEnemy->GetPos();
		}
	}

	// 距離計算
	m_distance = (m_playerPos - m_enemyPos).Length();

	// 10m未満になったらAIに委譲
	if (m_distance < m_targetDistance)
	{
		auto next = m_bossEnemy->GetBossEnemyAI()->DecideNext(m_bossEnemy);

		// 自分と同じ Run への再遷移は行わない（毎フレームアニメを張り直すのを防ぐ）
		m_bossEnemy->ChangeState(next);
		return;
	}

	// 追いかける（状態維持時のみ適用）
	Math::Vector3 dir = m_playerPos - m_enemyPos;
	dir.y = 0.0f;

	// ゼロベクトル対策
	if (dir.LengthSquared() > 1e-8f)
	{
		dir.Normalize();

		// LookRotationで正しい向きに（ゼロベクトルは渡さない）
		Math::Quaternion rot = Math::Quaternion::LookRotation(dir, Math::Vector3::Up);
		m_bossEnemy->SetRotation(rot);
	}

	m_bossEnemy->SetIsMoving(dir);
}

void BossEnemyState_Run::StateEnd()
{
}

void BossEnemyState_Run::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_Run));
	const auto& p = static_cast<const BossEnemyState_Run&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_targetDistance = p.m_targetDistance;
}

void BossEnemyState_Run::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
	ImGui::DragFloat(U8("ターゲット距離"), &m_targetDistance);
}

void BossEnemyState_Run::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("BossEnemyState_Run")) return;
	const auto& stateNode = js["BossEnemyState_Run"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
		if (enemyNode.contains("targetDistance")) m_targetDistance = enemyNode["targetDistance"].get<float>();
	}
}

void BossEnemyState_Run::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("BossEnemyState_Run")) js["BossEnemyState_Run"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_Run"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["BossEnemy"]["targetDistance"] = m_targetDistance;
}
