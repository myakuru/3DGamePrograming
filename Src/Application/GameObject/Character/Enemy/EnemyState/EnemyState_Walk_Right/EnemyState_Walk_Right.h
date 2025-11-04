#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Walk_Right : public EnemyStateBase
{
public:
	EnemyState_Walk_Right() = default;
	~EnemyState_Walk_Right()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;
};