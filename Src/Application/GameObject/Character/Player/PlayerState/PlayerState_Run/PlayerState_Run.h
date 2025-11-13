#pragma once
#include"../PlayerState.h"
class PlayerState_Run :public PlayerStateBase
{
public:
	PlayerState_Run() = default;
	~PlayerState_Run() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	// キー入力関連
	bool UpdateMoveInput();

};