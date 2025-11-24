#pragma once
#include"../PlayerState.h"
class ESkillEffect;
class PlayerState_Skill :public PlayerStateBase
{
public:
	PlayerState_Skill() = default;
	~PlayerState_Skill() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;


	std::weak_ptr<ESkillEffect> m_effect;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		assert(typeid(other) == typeid(PlayerState_Skill));
		const auto& p = static_cast<const PlayerState_Skill&>(other);
		m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	}

	void ExposeParametersImGui() override
	{
		m_stateParameter.ExposeImGui();
	}
	void LoadParametersJson(const nlohmann::json& js) override
	{
		if (!js.contains("PlayerState_Skill")) return;
		const auto& stateNode = js["PlayerState_Skill"];
		if (stateNode.contains("Player"))
		{
			m_stateParameter.LoadJson(stateNode["Player"]);
		}
	}
	void SaveParametersJson(nlohmann::json& js) const override
	{
		m_stateParameter.SaveJson(js["PlayerState_Skill"]);
	}


	bool m_flag = false; // 攻撃フラグ
};