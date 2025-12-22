#pragma once
#include"../EnemyStateBase.h"

class EnemyState_Attack2 : public EnemyStateBase
{
public:
	EnemyState_Attack2() = default;
	~EnemyState_Attack2()override = default;

	void StateStart(RedEnemy* _owner) override;
	void StateUpdate(RedEnemy* _owner) override;
	void StateEnd(RedEnemy* _owner) override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

private:

};