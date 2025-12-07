#pragma once
#include"../EnemyStateBase.h"
class EnemState_Attack3 : public EnemyStateBase
{
public:
	EnemState_Attack3() = default;
	~EnemState_Attack3() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;
};