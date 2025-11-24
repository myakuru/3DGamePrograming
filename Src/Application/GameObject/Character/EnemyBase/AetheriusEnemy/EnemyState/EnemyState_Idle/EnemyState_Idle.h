#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Idle : public EnemyStateBase
{
public:
	EnemyState_Idle() = default;
	~EnemyState_Idle()override = default;

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};