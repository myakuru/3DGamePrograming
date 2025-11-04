#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Hit : public EnemyStateBase
{
public:
	EnemyState_Hit() = default;
	~EnemyState_Hit()override = default;


private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};