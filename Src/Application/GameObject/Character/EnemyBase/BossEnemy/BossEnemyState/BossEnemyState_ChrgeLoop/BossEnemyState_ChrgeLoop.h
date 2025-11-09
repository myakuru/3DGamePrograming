#pragma once
#include"../BossEnemyState.h"
class BossEnemyState_ChrgeLoop : public BossEnemyStateBase
{
public:
	BossEnemyState_ChrgeLoop() = default;
	~BossEnemyState_ChrgeLoop()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};