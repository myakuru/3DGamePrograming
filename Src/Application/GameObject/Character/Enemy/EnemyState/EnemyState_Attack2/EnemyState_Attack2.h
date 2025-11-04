#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Attack2 : public EnemyStateBase
{
public:
	EnemyState_Attack2() = default;
	~EnemyState_Attack2()override = default;

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};