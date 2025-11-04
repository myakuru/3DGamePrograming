#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Death : public EnemyStateBase
{
public:
	EnemyState_Death() = default;
	~EnemyState_Death()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;
};