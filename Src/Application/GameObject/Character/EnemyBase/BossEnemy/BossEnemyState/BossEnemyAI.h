#pragma once

class BossEnemy;
class BossEnemyStateBase;

class BossEnemyAI
{
public:
	// 次の行動を決定してステートを生成
	static std::shared_ptr<BossEnemyStateBase> DecideNext(BossEnemy* boss);
};