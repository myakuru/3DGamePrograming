#pragma once
#include"../EnemyStateBase.h"
class AetheriusEnemy_Attack_2Effect;

class EnemyState_Attack2 : public EnemyStateBase
{
public:
	EnemyState_Attack2() = default;
	~EnemyState_Attack2()override = default;

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

private:

	std::weak_ptr<AetheriusEnemy_Attack_2Effect> m_attack2Effect;

};