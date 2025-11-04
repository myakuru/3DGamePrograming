#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Run : public BossEnemyStateBase
{
public:
	BossEnemyState_Run() = default;
	~BossEnemyState_Run()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};