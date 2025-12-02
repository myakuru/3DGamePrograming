#pragma once
#include"../../../../../MyFramework/State/StateBase/StateBase.h"
#include"../PlayerCamera.h"
class PlayerCameraState : public StateBase
{
public:
	PlayerCameraState() = default;
	~PlayerCameraState() override = default;

	void SetPlayerCamera(PlayerCamera* camera) { m_playerCamera = camera; }

	void ExposeParametersImGui() override {}
	// JSON 読み込み
	void LoadParametersJson(const nlohmann::json& _json) override {}
	// 保存
	void SaveParametersJson(nlohmann::json& _json) const override {}
	// ImGuiで編集した変数を実行時反映させるための関数
	virtual void ApplyFromConfig(const PlayerCameraState& other) {}

protected:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	PlayerCamera* m_playerCamera = nullptr;


	Math::Vector3 m_startFollow = Math::Vector3::Zero;
	Math::Vector3 m_endFollow = Math::Vector3::Zero;
	Math::Vector3 m_cameraRotation = Math::Vector3::Zero;
	Math::Vector2 m_noiseStrength = Math::Vector2::Zero;
	Math::Vector2 m_enableNoiseRandom = Math::Vector2::Zero;
	const float m_changeStateTime = 1.0f;
};