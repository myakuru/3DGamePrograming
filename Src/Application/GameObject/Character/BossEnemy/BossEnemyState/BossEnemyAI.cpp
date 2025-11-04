#include "BossEnemyAI.h"
#include "../BossEnemy.h"

// 次ステートで必要な派生をinclude
#include "../BossEnemyState/BossEnemyState_Run/BossEnemyState_Run.h"
#include "../BossEnemyState/BossEnemyState_Idle/BossEnemyState_Idle.h"
#include "../BossEnemyState/BossEnemyState_Attack_L/BossEnemyState_Attack_L.h"
#include "../BossEnemyState/BossEnemyState_Attack_R/BossEnemyState_Attack_R.h"
#include "../BossEnemyState/BossEnemyState_WaterAttack/BossEnemyState_WaterAttack.h"
#include"../BossEnemyState/BossEnemyState_WaterFall/BossEnemyState_WaterFall.h"
#include"../BossEnemyState/BossEnemyState_Dodge/BossEnemyState_Dodge.h"
#include"../../Player/Player.h"

#include "../../../../main.h"

static float GetDistanceToPlayer(BossEnemy* boss)
{
	if (!boss) return FLT_MAX;
	auto wp = boss->GetPlayerWeakPtr();
	auto sp = wp.lock();
	if (!sp) return FLT_MAX;
	return (sp->GetPos() - boss->GetPos()).Length();
}

std::shared_ptr<BossEnemyStateBase> BossEnemyAI::DecideNext(BossEnemy* boss)
{
	// フォールバックはIdle
	auto FallbackIdle = []() { return std::make_shared<BossEnemyState_Idle>(); };

	if (!boss) return FallbackIdle();

	const float dist = GetDistanceToPlayer(boss);

	// 安全策: プレイヤー未取得ならIdle
	if (dist == FLT_MAX) return FallbackIdle();

	// しきい値
	const float kMeleeRange = 4.0f;   // 近接可能距離(4mに拡張)
	const float kChaseRange = 20.0f;  // 追尾へ切替距離

	// 特殊条件: 体力半分以下でDodge優先
	//if (boss->GetStatus().hp <= boss->GetStatus().maxHp / 2)
	//{
	//	// 絶対にDodgeする
	//	boss->SetLastAction(BossEnemy::ActionType::Dodge);
	//	return std::make_shared<BossEnemyState_Dodge>();
	//}

	// ジャスト回避に対するカウンター（CD中以外）
	if (boss->GetJustAvoidSuccess() && boss->GetWaterCooldown() <= 0.0f)
	{
		boss->SetJustAvoidSuccess(false);      // 消費
		boss->SetWaterCooldown(8.0f);          // 少し長めのCD
		boss->SetLastAction(BossEnemy::ActionType::Water);
		return std::make_shared<BossEnemyState_WaterAttack>();
	}

	// 遠距離: 追尾
	if (dist > kChaseRange)
	{
		boss->SetLastAction(BossEnemy::ActionType::Run);
		return std::make_shared<BossEnemyState_Run>();
	}

	// 中距離: たまに水攻撃、なければ追尾
	if (dist >= kMeleeRange)
	{
		if (boss->GetWaterCooldown() <= 0.0f)
		{
			// 40%の確率で水攻撃
			if (KdRandom::GetInt(0, 99) < 40)
			{
				boss->SetWaterCooldown(20.0f);
				boss->SetLastAction(BossEnemy::ActionType::Water);
				return std::make_shared<BossEnemyState_WaterAttack>();
			}
		}
		boss->SetLastAction(BossEnemy::ActionType::Run);
		return std::make_shared<BossEnemyState_Run>();
	}

	// WaterFallのクールダウンが終わっていれば優先
	if (boss->GetWaterFallCooldown() <= 0.0f)
	{
		boss->SetWaterFallCooldown(7.0f);
		boss->SetLastAction(BossEnemy::ActionType::WaterFall);
		return std::make_shared<BossEnemyState_WaterFall>();
	}

	// 近距離: 近接コンボ開始（必ずLから開始）
	if (boss->GetMeleeCooldown() <= 0.0f)
	{
		boss->SetMeleeCooldown(1.0f);
		boss->SetLastAction(BossEnemy::ActionType::AttackL);
		return std::make_shared<BossEnemyState_Attack_L>();
	}

	// 近距離だがクールダウン中 → Idleせず追尾を継続
	boss->SetLastAction(BossEnemy::ActionType::Run);
	return std::make_shared<BossEnemyState_Run>();
}