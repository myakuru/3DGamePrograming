#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Walk_Left : public EnemyStateBase
{
public:
	EnemyState_Walk_Left() = default;
	~EnemyState_Walk_Left() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};