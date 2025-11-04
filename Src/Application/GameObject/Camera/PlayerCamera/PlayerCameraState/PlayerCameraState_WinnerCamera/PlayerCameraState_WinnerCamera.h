#pragma once
#include"../PlayerCameraState.h"
class PlayerCameraState_WinnerCamera : public PlayerCameraState
{
public:
	PlayerCameraState_WinnerCamera() = default;
	~PlayerCameraState_WinnerCamera() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

};