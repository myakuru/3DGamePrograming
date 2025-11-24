#pragma once
#include"../PlayerState.h"
class EffectPlay;
class SmokeEffect;
class ShineEffect;
class PlayerState_ChargeLevel2 :public PlayerStateBase
{
public:
	PlayerState_ChargeLevel2() = default;
	~PlayerState_ChargeLevel2() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::weak_ptr<EffectPlay> m_effect;
	std::weak_ptr<SmokeEffect> m_smokeEffect;
	std::weak_ptr<ShineEffect> m_shineEffect;

	float m_startSlowMotionTime = 0.0f; // スローモーション開始時間
	float m_endSlowMotionTime = 0.1f;   // スローモーション終了時間
};