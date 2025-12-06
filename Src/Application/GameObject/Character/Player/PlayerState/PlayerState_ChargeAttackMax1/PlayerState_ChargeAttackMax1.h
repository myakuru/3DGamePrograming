#pragma once
#include"../PlayerState.h"
class PlayerState_ChargeAttackMax1 :public PlayerStateBase
{
public:
	PlayerState_ChargeAttackMax1() = default;
	~PlayerState_ChargeAttackMax1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};