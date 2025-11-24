#pragma once
#include"../BossEnemyState.h"
class BossEnemyState_ChrgeLoop : public BossEnemyStateBase
{
public:
	BossEnemyState_ChrgeLoop() = default;
	~BossEnemyState_ChrgeLoop()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	float m_chargeTime = 0.0f;

};