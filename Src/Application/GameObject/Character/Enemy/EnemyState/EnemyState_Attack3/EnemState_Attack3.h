#pragma once
#include"../EnemyStateBase.h"
class EnemState_Attack3 : public EnemyStateBase
{
public:
	EnemState_Attack3() = default;
	~EnemState_Attack3() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};