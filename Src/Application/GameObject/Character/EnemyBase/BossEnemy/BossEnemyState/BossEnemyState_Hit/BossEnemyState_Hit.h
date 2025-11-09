#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Hit : public BossEnemyStateBase
{
public:
	BossEnemyState_Hit() = default;
	~BossEnemyState_Hit()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};