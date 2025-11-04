#pragma once
#include "../PlayerCameraState.h"

class PlayerCameraState_IntroCamera : public PlayerCameraState
{
public:
	PlayerCameraState_IntroCamera() = default;
	~PlayerCameraState_IntroCamera() override = default;

private:
	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;
};