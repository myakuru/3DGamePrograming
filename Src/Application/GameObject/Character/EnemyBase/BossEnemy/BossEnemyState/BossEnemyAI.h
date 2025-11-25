#pragma once

class BossEnemy;
class BossEnemyStateBase;

class BossEnemyAI
{
public:
	// 次の行動を決定してステートを生成
	std::shared_ptr<BossEnemyStateBase> DecideNext(BossEnemy* boss);
	void JsonInput(const nlohmann::json& _json);
	void JsonSave();

	float GetDistanceToPlayer(BossEnemy* boss);

private:
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

	BossEnemyAISettings m_settings; // グローバル設定

};