#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Attack3 : public EnemyStateBase
{
public:
	EnemyState_Attack3() = default;
	~EnemyState_Attack3()override = default;

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};