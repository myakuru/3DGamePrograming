#include "BossEnemyAI.h"
#include "../BossEnemy.h"
#include "../BossEnemyState/BossEnemyState_Run/BossEnemyState_Run.h"
#include "../BossEnemyState/BossEnemyState_Idle/BossEnemyState_Idle.h"
#include "../BossEnemyState/BossEnemyState_Attack_L/BossEnemyState_Attack_L.h"
#include "../BossEnemyState/BossEnemyState_Attack_R/BossEnemyState_Attack_R.h"
#include "../BossEnemyState/BossEnemyState_WaterAttack/BossEnemyState_WaterAttack.h"
#include "../BossEnemyState/BossEnemyState_WaterFall/BossEnemyState_WaterFall.h"
#include "../BossEnemyState/BossEnemyState_Dodge/BossEnemyState_Dodge.h"
#include "Application/GameObject/Character/Player/Player.h"
#include "Application/Scene/SceneManager.h"

#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"

// ================== ヘルパー ==================
float BossEnemyAI::GetDistanceToPlayer(BossEnemy* boss)
{
	if (!boss) return m_settings.noTargetDistance;
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::PlayerLike, boss->GetPlayerList());

	for (const auto& weakPlayer : boss->GetPlayerList())
	{
		if (auto sp = weakPlayer.lock())
		{
			return (sp->GetPos() - boss->GetPos()).Length();
		}
	}
	return m_settings.noTargetDistance;
}

// ================== AI決定 ==================
std::shared_ptr<BossEnemyStateBase> BossEnemyAI::DecideNext(BossEnemy* boss)
{
	auto FallbackIdle = []() { return std::make_shared<BossEnemyState_Idle>(); };
	if (!boss) return FallbackIdle();

	const float dist = GetDistanceToPlayer(boss);

	// ジャスト回避カウンター
	if (boss->GetJustAvoidSuccess() && boss->GetWaterCooldown() <= 0.0f)
	{
		boss->SetJustAvoidSuccess(false);
		boss->SetWaterCooldown(m_settings.justAvoidWaterCooldown);
		boss->SetLastAction(BossEnemy::ActionType::Water);
		return std::make_shared<BossEnemyState_WaterAttack>();
	}

	// 遠距離: 追尾
	if (dist > m_settings.chaseRange)
	{
		boss->SetLastAction(BossEnemy::ActionType::Run);
		return std::make_shared<BossEnemyState_Run>();
	}

	// 中距離: 水攻撃判定
	if (dist >= m_settings.meleeRange)
	{
		if (boss->GetWaterCooldown() <= 0.0f)
		{
			const int threshold = static_cast<int>(m_settings.waterAttackProbability * m_settings.randomRangeMaxExclusive + 0.5f);
			const int roll = KdRandom::GetInt(0, m_settings.randomRangeMaxExclusive - 1);
			if (roll < std::clamp(threshold, 0, m_settings.randomRangeMaxExclusive))
			{
				boss->SetWaterCooldown(m_settings.waterAttackCooldown);
				boss->SetLastAction(BossEnemy::ActionType::Water);
				return std::make_shared<BossEnemyState_WaterAttack>();
			}
		}
		boss->SetLastAction(BossEnemy::ActionType::Run);
		return std::make_shared<BossEnemyState_Run>();
	}

	// WaterFall優先
	if (boss->GetWaterFallCooldown() <= 0.0f)
	{
		boss->SetWaterFallCooldown(m_settings.waterFallCooldown);
		boss->SetLastAction(BossEnemy::ActionType::WaterFall);
		return std::make_shared<BossEnemyState_WaterFall>();
	}

	// 近接開始
	if (boss->GetMeleeCooldown() <= 0.0f)
	{
		boss->SetMeleeCooldown(m_settings.meleeAttackCooldown);
		boss->SetLastAction(BossEnemy::ActionType::AttackL);
		return std::make_shared<BossEnemyState_Attack_L>();
	}

	// クールダウン中はRun継続
	boss->SetLastAction(BossEnemy::ActionType::Run);
	return std::make_shared<BossEnemyState_Run>();
}

// ================== JSON入力 ==================
void BossEnemyAI::JsonInput(const nlohmann::json& _json)
{
	if (!_json.is_object()) return;
	if (!_json.contains("BossEnemyAI")) return;

	const auto& node = _json["BossEnemyAI"];

	m_settings.chaseRange = node.value("chaseRange", m_settings.chaseRange);
	m_settings.meleeRange = node.value("meleeRange", m_settings.meleeRange);
	m_settings.justAvoidWaterCooldown = node.value("justAvoidWaterCooldown", m_settings.justAvoidWaterCooldown);
	m_settings.waterAttackProbability = node.value("waterAttackProbability", m_settings.waterAttackProbability);
	m_settings.waterAttackCooldown = node.value("waterAttackCooldown", m_settings.waterAttackCooldown);
	m_settings.waterFallCooldown = node.value("waterFallCooldown", m_settings.waterFallCooldown);
	m_settings.meleeAttackCooldown = node.value("meleeAttackCooldown", m_settings.meleeAttackCooldown);
	m_settings.randomRangeMaxExclusive = node.value("randomRangeMaxExclusive", m_settings.randomRangeMaxExclusive);
}

// ================== JSON保存 ==================
void BossEnemyAI::JsonSave()
{
	nlohmann::json root = nlohmann::json::object();
	auto& node = root["BossEnemyAI"];

	node["chaseRange"] = m_settings.chaseRange;
	node["meleeRange"] = m_settings.meleeRange;
	node["justAvoidWaterCooldown"] = m_settings.justAvoidWaterCooldown;
	node["waterAttackProbability"] = m_settings.waterAttackProbability;
	node["waterAttackCooldown"] = m_settings.waterAttackCooldown;
	node["waterFallCooldown"] = m_settings.waterFallCooldown;
	node["meleeAttackCooldown"] = m_settings.meleeAttackCooldown;
	node["randomRangeMaxExclusive"] = m_settings.randomRangeMaxExclusive;

	JSON_MANAGER.JsonSerialize(root, "Json/BossEnemyConfig/BossEnemyAI/BossEnemyAI");
}
