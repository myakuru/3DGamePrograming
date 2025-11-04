#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_WaterAttack_end : public BossEnemyStateBase
{
public:
	BossEnemyState_WaterAttack_end() = default;
	~BossEnemyState_WaterAttack_end()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};