#pragma once
#include"../PlayerState.h"

class PlayerState_AvoidAttack :public PlayerStateBase
{
public:
	PlayerState_AvoidAttack() = default;
	~PlayerState_AvoidAttack() override = default;

private:
	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;


	Math::Vector3 m_avoidDirection = Math::Vector3::Zero;
};