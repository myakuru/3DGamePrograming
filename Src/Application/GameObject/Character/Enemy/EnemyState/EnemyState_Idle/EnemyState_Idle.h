#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Idle : public EnemyStateBase
{
public:
	EnemyState_Idle() = default;
	~EnemyState_Idle()override = default;

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};