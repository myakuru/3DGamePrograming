#pragma once
#include"../BossEnemyState.h"
class BossWaterFallAttack;
class BossEnemyState_WaterFall : public BossEnemyStateBase
{
public:
	BossEnemyState_WaterFall() = default;
	~BossEnemyState_WaterFall()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::weak_ptr<BossWaterFallAttack> m_waterFallAttack;

};