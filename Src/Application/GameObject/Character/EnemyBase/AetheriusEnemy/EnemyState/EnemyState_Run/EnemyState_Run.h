#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Run : public EnemyStateBase
{
public:
	EnemyState_Run() = default;
	~EnemyState_Run()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};