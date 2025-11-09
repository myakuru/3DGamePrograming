#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossAttack_1stEffect;
class BossEnemyState_Attack_L : public BossEnemyStateBase
{
public:
	BossEnemyState_Attack_L() = default;
	~BossEnemyState_Attack_L()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;
	std::weak_ptr<BossAttack_1stEffect> m_attackEffect;

	int m_randomNum = 0;

};