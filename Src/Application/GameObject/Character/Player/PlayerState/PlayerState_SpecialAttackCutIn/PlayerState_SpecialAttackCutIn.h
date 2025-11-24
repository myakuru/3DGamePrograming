#pragma once
#include"../PlayerState.h"
class PlayerState_SpecialAttackCutIn :public PlayerStateBase
{
public:
	PlayerState_SpecialAttackCutIn() = default;
	~PlayerState_SpecialAttackCutIn() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	float m_cameraStartRotationSmooth = 5.0f;
	float m_cameraStartDistanceSmooth = 5.0f;
	Math::Vector3 m_cameraCutInOffset = { 0.0f,0.6f,-1.7f };
	Math::Vector3 m_cameraCutInRotation = { 10.0f,90.0f,5.0f };
	float m_cutInCameraTime = 0.6f;

};