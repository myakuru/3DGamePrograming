#pragma once
#include"../PlayerState.h"
class PlayerState_FowardAvoidFast :public PlayerStateBase
{
public:
	PlayerState_FowardAvoidFast() = default;
	~PlayerState_FowardAvoidFast() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	bool m_afterImagePlayed = false; // 残像エフェクトが再生されたかどうか

};