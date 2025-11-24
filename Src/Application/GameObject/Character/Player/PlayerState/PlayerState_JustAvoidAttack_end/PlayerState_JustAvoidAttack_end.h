#pragma once
#include"../PlayerState.h"
class JustAvoidAttackEffect;
class PlayerState_JustAvoidAttack_end :public PlayerStateBase
{
public:
	PlayerState_JustAvoidAttack_end() = default;
	~PlayerState_JustAvoidAttack_end() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::weak_ptr<JustAvoidAttackEffect> m_justAvoidAttackEffect;

};