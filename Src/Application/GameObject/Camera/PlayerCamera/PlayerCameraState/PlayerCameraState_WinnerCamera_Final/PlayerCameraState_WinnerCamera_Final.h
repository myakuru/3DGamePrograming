#pragma once
#include"../PlayerCameraState.h"
class PlayerCameraState_WinnerCamera_Final : public PlayerCameraState
{
public:
	PlayerCameraState_WinnerCamera_Final() = default;
	~PlayerCameraState_WinnerCamera_Final() override = default;

private:

	void StateStart(PlayerCamera* _owner) override;
	void StateUpdate(PlayerCamera* _owner) override;
	void StateEnd(PlayerCamera* _owner) override;

	void ApplyFromConfig(const PlayerCameraState& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};