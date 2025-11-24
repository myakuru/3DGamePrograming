#pragma once
#include"../BossEnemyState.h"
class BossEnemyState_ChargeEnd : public BossEnemyStateBase
{
public:
	BossEnemyState_ChargeEnd() = default;
	~BossEnemyState_ChargeEnd()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};