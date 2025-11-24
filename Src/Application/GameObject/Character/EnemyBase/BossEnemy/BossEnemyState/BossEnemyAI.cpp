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

struct BossEnemyAISettings
{
	float chaseRange = 0.0f;				// 追尾開始距離
	float meleeRange = 0.0f;				// 近接開始距離
	float justAvoidWaterCooldown = 0.0f;	// ジャスト回避後の水攻撃クールダウン
	float waterAttackProbability = 0.0f;	// 中距離で水攻撃を選ぶ確率(0～1)
	float waterAttackCooldown = 0.0f;		// 水攻撃使用後クールダウン
	float waterFallCooldown = 0.0f;			// WaterFall攻撃クールダウン
	float meleeAttackCooldown = 0.0f;		// 近接攻撃（L開始）クールダウン
	int   randomRangeMaxExclusive = 0;		// 乱数最大(排他的) 0～99
	float noTargetDistance = FLT_MAX;		// ターゲット無し距離
};

BossEnemyAISettings g_settings; // グローバル設定

// ================== ヘルパー ==================
static float GetDistanceToPlayer(BossEnemy* boss)
{
	if (!boss) return g_settings.noTargetDistance;
	SceneManager::Instance().GetObjectWeakPtrListByTag(ObjTag::PlayerLike, boss->GetPlayerList());

	for (const auto& weakPlayer : boss->GetPlayerList())
	{
		if (auto sp = weakPlayer.lock())
		{
			return (sp->GetPos() - boss->GetPos()).Length();
		}
	}
	return g_settings.noTargetDistance;
}

// ================== AI決定 ==================
std::shared_ptr<BossEnemyStateBase> BossEnemyAI::DecideNext(BossEnemy* boss)
{
	auto FallbackIdle = []() { return std::make_shared<BossEnemyState_Idle>(); };
	if (!boss) return FallbackIdle();

	const float dist = GetDistanceToPlayer(boss);
	auto& s = g_settings;

	// ジャスト回避カウンター
	if (boss->GetJustAvoidSuccess() && boss->GetWaterCooldown() <= 0.0f)
	{
		boss->SetJustAvoidSuccess(false);
		boss->SetWaterCooldown(s.justAvoidWaterCooldown);
		boss->SetLastAction(BossEnemy::ActionType::Water);
		return std::make_shared<BossEnemyState_WaterAttack>();
	}

	// 遠距離: 追尾
	if (dist > s.chaseRange)
	{
		boss->SetLastAction(BossEnemy::ActionType::Run);
		return std::make_shared<BossEnemyState_Run>();
	}

	// 中距離: 水攻撃判定
	if (dist >= s.meleeRange)
	{
		if (boss->GetWaterCooldown() <= 0.0f)
		{
			const int threshold = static_cast<int>(s.waterAttackProbability * s.randomRangeMaxExclusive + 0.5f);
			const int roll = KdRandom::GetInt(0, s.randomRangeMaxExclusive - 1);
			if (roll < std::clamp(threshold, 0, s.randomRangeMaxExclusive))
			{
				boss->SetWaterCooldown(s.waterAttackCooldown);
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
		boss->SetWaterFallCooldown(s.waterFallCooldown);
		boss->SetLastAction(BossEnemy::ActionType::WaterFall);
		return std::make_shared<BossEnemyState_WaterFall>();
	}

	// 近接開始
	if (boss->GetMeleeCooldown() <= 0.0f)
	{
		boss->SetMeleeCooldown(s.meleeAttackCooldown);
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

	g_settings.chaseRange = node.value("chaseRange", g_settings.chaseRange);
	g_settings.meleeRange = node.value("meleeRange", g_settings.meleeRange);
	g_settings.justAvoidWaterCooldown = node.value("justAvoidWaterCooldown", g_settings.justAvoidWaterCooldown);
	g_settings.waterAttackProbability = node.value("waterAttackProbability", g_settings.waterAttackProbability);
	g_settings.waterAttackCooldown = node.value("waterAttackCooldown", g_settings.waterAttackCooldown);
	g_settings.waterFallCooldown = node.value("waterFallCooldown", g_settings.waterFallCooldown);
	g_settings.meleeAttackCooldown = node.value("meleeAttackCooldown", g_settings.meleeAttackCooldown);
	g_settings.randomRangeMaxExclusive = node.value("randomRangeMaxExclusive", g_settings.randomRangeMaxExclusive);
}

// ================== JSON保存 ==================
void BossEnemyAI::JsonSave()
{
	nlohmann::json root = nlohmann::json::object();
	auto& node = root["BossEnemyAI"];

	node["chaseRange"] = g_settings.chaseRange;
	node["meleeRange"] = g_settings.meleeRange;
	node["justAvoidWaterCooldown"] = g_settings.justAvoidWaterCooldown;
	node["waterAttackProbability"] = g_settings.waterAttackProbability;
	node["waterAttackCooldown"] = g_settings.waterAttackCooldown;
	node["waterFallCooldown"] = g_settings.waterFallCooldown;
	node["meleeAttackCooldown"] = g_settings.meleeAttackCooldown;
	node["randomRangeMaxExclusive"] = g_settings.randomRangeMaxExclusive;

	JSON_MANAGER.JsonSerialize(root, "Json/BossEnemyConfig/BossEnemyAI/BossEnemyAI");
}