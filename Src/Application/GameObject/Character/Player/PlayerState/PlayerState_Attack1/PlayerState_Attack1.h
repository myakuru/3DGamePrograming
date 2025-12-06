#pragma once
#include"../PlayerState.h"

class PlayerState_Attack1 : public PlayerStateBase
{
public:
	PlayerState_Attack1();
	~PlayerState_Attack1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const PlayerStateBase& other) override;
	void ExposeParametersImGui() override;

	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	bool m_flag = false; // 攻撃フラグ
	bool m_effectOnce = false; // エフェクト一回だけ再生するフラグ
};