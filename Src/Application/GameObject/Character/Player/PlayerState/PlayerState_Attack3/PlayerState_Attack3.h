#pragma once
#include"../PlayerState.h"
class SlashAttack2Effect;
class PlayerState_Attack3 :public PlayerStateBase
{
public:
	PlayerState_Attack3() = default;
	~PlayerState_Attack3() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		assert(typeid(other) == typeid(PlayerState_Attack3));
		const auto& p = static_cast<const PlayerState_Attack3&>(other);
		m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	}

	void ExposeParametersImGui() override
	{
		m_stateParameter.ExposeImGui();
	}
	void LoadParametersJson(const nlohmann::json& js) override
	{
		if (!js.contains("PlayerState_Attack3")) return;
		const auto& stateNode = js["PlayerState_Attack3"];
		if (stateNode.contains("Player"))
		{
			m_stateParameter.LoadJson(stateNode["Player"]);
		}
	}
	void SaveParametersJson(nlohmann::json& js) const override
	{
		m_stateParameter.SaveJson(js["PlayerState_Attack3"]);
	}

	std::weak_ptr<SlashAttack2Effect> m_slashEffect;

};