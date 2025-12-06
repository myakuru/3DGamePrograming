#pragma once
#include"../PlayerState.h"

class PlayerState_Attack : public PlayerStateBase
{
public:
	PlayerState_Attack();
	~PlayerState_Attack() override = default;

	void ApplyFromConfig(const PlayerStateBase& other) override;

private:
	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};