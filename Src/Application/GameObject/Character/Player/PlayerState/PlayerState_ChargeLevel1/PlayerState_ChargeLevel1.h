
#pragma once
#include"../PlayerState.h"
class PlayerState_ChargeLevel1 :public PlayerStateBase
{
public:
	PlayerState_ChargeLevel1() = default;
	~PlayerState_ChargeLevel1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	float m_startSlowMotionTime = 0.0f; // スローモーション開始時間
	float m_endSlowMotionTime = 0.1f;   // スローモーション終了時間

};