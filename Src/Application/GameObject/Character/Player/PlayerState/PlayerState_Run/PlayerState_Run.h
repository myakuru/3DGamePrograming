#pragma once
#include"../PlayerState.h"
class PlayerState_Run :public PlayerStateBase
{
public:
	PlayerState_Run() = default;
	~PlayerState_Run() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	// キー入力関連
	bool UpdateMoveInput();

	void ApplyFromConfig(const PlayerStateBase& other) override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};