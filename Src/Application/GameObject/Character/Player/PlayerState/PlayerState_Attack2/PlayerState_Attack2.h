#pragma once
#include"../PlayerState.h"
class AttacEffect1;
class PlayerState_Attack2 : public PlayerStateBase
{
public:
	PlayerState_Attack2() = default;
	~PlayerState_Attack2() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		assert(typeid(other) == typeid(PlayerState_Attack2));
		const auto& p = static_cast<const PlayerState_Attack2&>(other);
		m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	}

	void ExposeParametersImGui() override
	{
		m_stateParameter.ExposeImGui();
	}
	void LoadParametersJson(const nlohmann::json& js) override
	{
		if (!js.contains("PlayerState_Attack2")) return;
		const auto& stateNode = js["PlayerState_Attack2"];
		if (stateNode.contains("Player"))
		{
			m_stateParameter.LoadJson(stateNode["Player"]);
		}
	}
	void SaveParametersJson(nlohmann::json& js) const override
	{
		m_stateParameter.SaveJson(js["PlayerState_Attack2"]);
	}

	std::weak_ptr<AttacEffect1> m_slashEffect;


	bool m_flag = false; // 攻撃フラグ

};