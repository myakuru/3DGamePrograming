#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_ChargeStart : public BossEnemyStateBase
{
public:
	BossEnemyState_ChargeStart() = default;
	~BossEnemyState_ChargeStart()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};