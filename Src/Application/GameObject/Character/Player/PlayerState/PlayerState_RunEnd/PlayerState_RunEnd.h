#pragma once
#include"../PlayerState.h"
class PlayerState_RunEnd :public PlayerStateBase
{
public:
	PlayerState_RunEnd() = default;
	~PlayerState_RunEnd() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	bool UpdateMoveInput();

	void ApplyFromConfig(const PlayerStateBase& other) override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};