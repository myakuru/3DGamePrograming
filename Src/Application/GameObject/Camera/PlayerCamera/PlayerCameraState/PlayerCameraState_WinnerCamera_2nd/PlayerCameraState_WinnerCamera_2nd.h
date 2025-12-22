#pragma once
#include"../PlayerCameraState.h"
class PlayerCameraState_WinnerCamera_2nd : public PlayerCameraState
{
public:
	PlayerCameraState_WinnerCamera_2nd() = default;
	~PlayerCameraState_WinnerCamera_2nd() override = default;

private:

	void StateStart(PlayerCamera* _owner) override;
	void StateUpdate(PlayerCamera* _owner) override;
	void StateEnd(PlayerCamera* _owner) override;

	void ApplyFromConfig(const PlayerCameraState& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};