#pragma once
#include"../PlayerState.h"
class PlayerState_ChargeAttackMax3 :public PlayerStateBase
{
public:
	PlayerState_ChargeAttackMax3() = default;
	~PlayerState_ChargeAttackMax3() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};