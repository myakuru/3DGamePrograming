#pragma once
#include"../PlayerState.h"
class TrailEffect;
class SwordFlash;

class PlayerState_Attack : public PlayerStateBase
{
public:
	PlayerState_Attack() = default;
	~PlayerState_Attack() override = default;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		assert(typeid(other) == typeid(PlayerState_Attack));
		const auto& p = static_cast<const PlayerState_Attack&>(other);
		m_stateParameter = p.m_stateParameter; // 構造体一括コピー
	}

private:
	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::weak_ptr<TrailEffect> m_trailEffect;
	std::weak_ptr<SwordFlash>  m_slashEffect;

};