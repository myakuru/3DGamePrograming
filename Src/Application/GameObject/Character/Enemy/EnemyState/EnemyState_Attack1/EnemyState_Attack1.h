#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Attack1 : public EnemyStateBase
{
public:
	EnemyState_Attack1() = default;
	~EnemyState_Attack1()override = default;

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};