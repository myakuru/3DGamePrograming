#pragma once
#include"../PlayerState.h"
class SpecialAttack;
class PlayerState_SpecialAttack :public PlayerStateBase
{
public:
	PlayerState_SpecialAttack() = default;
	~PlayerState_SpecialAttack() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;


	std::weak_ptr<SpecialAttack> m_specialAttackEffect;

	bool m_playSound = false;	// 効果音再生フラグ

	Math::Vector3 m_lastCameraPos = Math::Vector3::Zero;

};