#pragma once
#include"../PlayerState.h"
class PlayerState_SheathKatana :public PlayerStateBase
{
public:
	PlayerState_SheathKatana() = default;
	~PlayerState_SheathKatana() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::shared_ptr<KdSoundInstance> m_SheathKatanaSound = nullptr;

	void ApplyFromConfig(const PlayerStateBase& other) override;

	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};