#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Run : public EnemyStateBase
{
public:
	EnemyState_Run() = default;
	~EnemyState_Run()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};