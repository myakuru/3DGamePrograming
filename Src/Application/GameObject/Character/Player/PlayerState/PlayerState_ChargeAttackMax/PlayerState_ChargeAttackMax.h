#pragma once
#include"../PlayerState.h"
class PlayerState_ChargeAttackMax :public PlayerStateBase
{
public:
	PlayerState_ChargeAttackMax() = default;
	~PlayerState_ChargeAttackMax() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;



};