#pragma once
#include"../PlayerCameraState.h"

class Player;
class PlayerCameraState_LookPlayer : public PlayerCameraState
{
public:
	PlayerCameraState_LookPlayer() = default;
	~PlayerCameraState_LookPlayer() override = default;

private:

	void StateStart(PlayerCamera* _owner) override;
	void StateUpdate(PlayerCamera* _owner) override;
	void StateEnd(PlayerCamera* _owner) override;

};