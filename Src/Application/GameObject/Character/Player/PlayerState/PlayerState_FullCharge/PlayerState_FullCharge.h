#include"../PlayerState.h"
#pragma once
class PlayerState_FullCharge :public PlayerStateBase
{ 
public:
	PlayerState_FullCharge() = default;
	~PlayerState_FullCharge() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;
};
