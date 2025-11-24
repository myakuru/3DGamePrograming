#pragma once
#include"../PlayerState.h"
class SpeedAttackEffect;
class PlayerState_AvoidAttack :public PlayerStateBase
{
public:
	PlayerState_AvoidAttack() = default;
	~PlayerState_AvoidAttack() override = default;

private:
	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		assert(typeid(other) == typeid(PlayerState_AvoidAttack));
		const auto& p = static_cast<const PlayerState_AvoidAttack&>(other);
		m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	}

	void ExposeParametersImGui() override
	{
		m_stateParameter.ExposeImGui();
	}
	void LoadParametersJson(const nlohmann::json& js) override
	{
		if (!js.contains("PlayerState_AvoidAttack")) return;
		const auto& stateNode = js["PlayerState_AvoidAttack"];
		if (stateNode.contains("Player"))
		{
			m_stateParameter.LoadJson(stateNode["Player"]);
		}
	}
	void SaveParametersJson(nlohmann::json& js) const override
	{
		m_stateParameter.SaveJson(js["PlayerState_AvoidAttack"]);
	}

	Math::Vector3 m_avoidDirection = Math::Vector3::Zero;

	std::weak_ptr<SpeedAttackEffect> m_effect;
};