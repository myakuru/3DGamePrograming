#pragma once
#include"../PlayerState.h"
class PlayerState_RunStart :public PlayerStateBase
{
public:
	PlayerState_RunStart() = default;
	~PlayerState_RunStart() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};