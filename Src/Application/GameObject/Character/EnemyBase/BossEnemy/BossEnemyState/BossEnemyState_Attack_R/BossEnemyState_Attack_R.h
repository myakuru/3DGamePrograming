#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Attack_R : public BossEnemyStateBase
{
public:
	BossEnemyState_Attack_R() = default;
	~BossEnemyState_Attack_R()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};