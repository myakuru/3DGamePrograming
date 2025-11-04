#pragma once
#include"../PlayerCameraState.h"
class PlayerCameraState_LookPlayer : public PlayerCameraState
{
public:
	PlayerCameraState_LookPlayer() = default;
	~PlayerCameraState_LookPlayer() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};