#pragma once
#include"../PlayerState.h"
class PlayerState_Attack2 : public PlayerStateBase
{
public:
	PlayerState_Attack2() = default;
	~PlayerState_Attack2() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};