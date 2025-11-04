#pragma once
#include"../../../../../MyFramework/State/StateBase/StateBase.h"
#include"../PlayerCamera.h"
class PlayerCameraState : public StateBase
{
public:
	PlayerCameraState() = default;
	~PlayerCameraState() override = default;

	void SetPlayerCamera(PlayerCamera* camera) { m_playerCamera = camera; }

protected:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	PlayerCamera* m_playerCamera = nullptr;

};