#pragma once
#include"../PlayerState.h"
class GroundFreezes;
class Rotation;
class PlayerState_Attack4 :public PlayerStateBase
{
public:
	PlayerState_Attack4() = default;
	~PlayerState_Attack4() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		assert(typeid(other) == typeid(PlayerState_Attack4));
		const auto& p = static_cast<const PlayerState_Attack4&>(other);
		m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	}

	void ExposeParametersImGui() override
	{
		m_stateParameter.ExposeImGui();
	}
	void LoadParametersJson(const nlohmann::json& js) override
	{
		if (!js.contains("PlayerState_Attack4")) return;
		const auto& stateNode = js["PlayerState_Attack4"];
		if (stateNode.contains("Player"))
		{
			m_stateParameter.LoadJson(stateNode["Player"]);
		}
	}
	void SaveParametersJson(nlohmann::json& js) const override
	{
		m_stateParameter.SaveJson(js["PlayerState_Attack4"]);
	}

	std::weak_ptr<GroundFreezes> m_groundFreezes;
	std::weak_ptr<Rotation> m_rotationEffect;
};