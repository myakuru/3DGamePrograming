#pragma once
#include"../PlayerState.h"
class PlayerState_SpecialAttack1 :public PlayerStateBase
{
public:

	PlayerState_SpecialAttack1() = default;
	~PlayerState_SpecialAttack1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	bool m_effectOnce = false;       // 再生要求を出したか
	bool m_effectStarted = false;    // 実際に再生が始まったか（IsPlaying が最初に true になった）

	float m_yawRad = 0.0f;

	float m_yawDeg = 0.0f;

	bool m_playSound = false;

	void ApplyFromConfig(const PlayerStateBase& other) override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};