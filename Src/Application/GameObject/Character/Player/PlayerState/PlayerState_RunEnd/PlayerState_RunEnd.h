#pragma once
#include"../PlayerState.h"
class PlayerState_RunEnd :public PlayerStateBase
{
public:
	PlayerState_RunEnd() = default;
	~PlayerState_RunEnd() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;
};