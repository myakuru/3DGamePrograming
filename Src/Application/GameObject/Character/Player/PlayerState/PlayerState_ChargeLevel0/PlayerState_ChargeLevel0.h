#include"../PlayerState.h"
#pragma once
class PlayerState_ChargeLevel0 :public PlayerStateBase
{ 
public:
	PlayerState_ChargeLevel0() = default;
	~PlayerState_ChargeLevel0() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;
};
