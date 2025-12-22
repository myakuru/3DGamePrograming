#pragma once
#include"../BossEnemyState.h"
class BossEnemyState_ChargeLoop : public BossEnemyStateBase
{
public:
	BossEnemyState_ChargeLoop() = default;
	~BossEnemyState_ChargeLoop()override = default;

private:

	void StateStart(BossEnemy* _owner) override;
	void StateUpdate(BossEnemy* _owner) override;
	void StateEnd(BossEnemy* _owner) override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	float m_chargeTime = 0.0f;

};