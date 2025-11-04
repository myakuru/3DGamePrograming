#pragma once
#include"../PlayerState.h"
class PlayerState_SpecialAttackCutIn :public PlayerStateBase
{
public:
	PlayerState_SpecialAttackCutIn() = default;
	~PlayerState_SpecialAttackCutIn() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};