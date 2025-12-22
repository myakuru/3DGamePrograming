#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Walk_Right : public EnemyStateBase
{
public:
	EnemyState_Walk_Right() = default;
	~EnemyState_Walk_Right()override = default;

	void StateStart(RedEnemy* _owner) override;
	void StateUpdate(RedEnemy* _owner) override;
	void StateEnd(RedEnemy* _owner) override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;
};