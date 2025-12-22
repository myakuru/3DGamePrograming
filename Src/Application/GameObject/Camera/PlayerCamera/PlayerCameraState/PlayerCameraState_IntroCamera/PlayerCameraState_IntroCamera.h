#pragma once
#include "../PlayerCameraState.h"

class PlayerCameraState_IntroCamera : public PlayerCameraState
{
public:
	PlayerCameraState_IntroCamera() = default;
	~PlayerCameraState_IntroCamera() override = default;

private:
	void StateStart(PlayerCamera* _owner) override;
	void StateUpdate(PlayerCamera* _owner) override;
	void StateEnd(PlayerCamera* _owner	) override;

	bool m_started = false;

	float m_timer = 0.0f;
};