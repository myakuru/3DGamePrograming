#pragma once
#include"../PlayerState.h"
class PlayerState_BackWordAvoid :public PlayerStateBase
{
public:
	PlayerState_BackWordAvoid() = default;
	~PlayerState_BackWordAvoid() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	bool m_afterImagePlayed = false;

	Math::Vector3 m_startCameraOffset = { 0.0f,1.0f,-4.5f };
	Math::Vector3 m_startBossCameraOffset = { 0.0f,1.0f,-7.5f };
	Math::Vector3 m_justAvoidCameraOffset = { 0.0f,0.7f,-1.2f };

};