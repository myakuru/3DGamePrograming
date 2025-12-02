#pragma once
#include"../PlayerCameraState.h"
class PlayerCameraState_WinnerCamera_Final : public PlayerCameraState
{
public:
	PlayerCameraState_WinnerCamera_Final() = default;
	~PlayerCameraState_WinnerCamera_Final() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerCameraState& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};