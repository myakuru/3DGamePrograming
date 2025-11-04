#pragma once
#include "../PlayerState.h"
class PlayerState_ChargeAttackMax2 :public PlayerStateBase
{
public:
	PlayerState_ChargeAttackMax2() = default;
	~PlayerState_ChargeAttackMax2() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;


};