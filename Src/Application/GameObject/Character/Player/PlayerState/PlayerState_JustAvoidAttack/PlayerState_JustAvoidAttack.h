#pragma once
#include"../PlayerState.h"
class JustAvoidAttackEffect1;
class PlayerState_JustAvoidAttack :public PlayerStateBase
{
public:
	PlayerState_JustAvoidAttack() = default;
	~PlayerState_JustAvoidAttack() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::weak_ptr<JustAvoidAttackEffect1> m_justAvoidAttackEffect;

	float m_overshootDist = 4.0f;
	Math::Vector3 m_dashDirection = Math::Vector3::Zero;

};