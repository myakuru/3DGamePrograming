#pragma once
#include"../PlayerState.h"
class Rotation;

class PlayerState_Attack1 : public PlayerStateBase
{
public:
	PlayerState_Attack1() = default;
	~PlayerState_Attack1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		assert(typeid(other) == typeid(PlayerState_Attack1));
		const auto& p = static_cast<const PlayerState_Attack1&>(other);
		m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	}

	void ExposeParametersImGui() override
	{
		m_stateParameter.ExposeImGui();
	}
	void LoadParametersJson(const nlohmann::json& js) override
	{
		if (!js.contains("PlayerState_Attack1")) return;
		const auto& stateNode = js["PlayerState_Attack1"];
		if (stateNode.contains("Player"))
		{
			m_stateParameter.LoadJson(stateNode["Player"]);
		}
	}
	void SaveParametersJson(nlohmann::json& js) const override
	{
		m_stateParameter.SaveJson(js["PlayerState_Attack1"]);
	}

	std::weak_ptr<Rotation> m_effect; // エフェクト

	bool m_flag = false; // 攻撃フラグ
	bool m_effectOnce = false; // エフェクト一回だけ再生するフラグ
};