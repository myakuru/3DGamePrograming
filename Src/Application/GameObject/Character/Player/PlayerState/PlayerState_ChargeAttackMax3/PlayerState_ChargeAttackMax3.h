#pragma once
#include"../PlayerState.h"
class ChargeAttackEffect_end;
class PlayerState_ChargeAttackMax3 :public PlayerStateBase
{
public:
	PlayerState_ChargeAttackMax3() = default;
	~PlayerState_ChargeAttackMax3() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		assert(typeid(other) == typeid(PlayerState_ChargeAttackMax3));
		const auto& p = static_cast<const PlayerState_ChargeAttackMax3&>(other);
		m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	}

	void ExposeParametersImGui() override
	{
		m_stateParameter.ExposeImGui();
	}
	void LoadParametersJson(const nlohmann::json& js) override
	{
		if (!js.contains("PlayerState_ChargeAttackMax3")) return;
		const auto& stateNode = js["PlayerState_ChargeAttackMax3"];
		if (stateNode.contains("Player"))
		{
			m_stateParameter.LoadJson(stateNode["Player"]);
		}
	}
	void SaveParametersJson(nlohmann::json& js) const override
	{
		m_stateParameter.SaveJson(js["PlayerState_ChargeAttackMax3"]);
	}

	std::weak_ptr<ChargeAttackEffect_end> m_effect;

};