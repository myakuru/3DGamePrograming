#pragma once
#include"../BossEnemyState.h"
class BossEnemyState_Attack_R : public BossEnemyStateBase
{
public:
	BossEnemyState_Attack_R() = default;
	~BossEnemyState_Attack_R()override = default;

private:

	void StateStart(BossEnemy* _owner) override;
	void StateUpdate(BossEnemy* _owner) override;
	void StateEnd(BossEnemy* _owner) override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};