#pragma once
#include"../EnemyStateBase.h"
class AetheriusEnemy_Attack_1Effect;

class EnemyState_Attack1 : public EnemyStateBase
{
public:
	EnemyState_Attack1() = default;
	~EnemyState_Attack1()override = default;

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::weak_ptr<AetheriusEnemy_Attack_1Effect> m_attackEffect; // 攻撃エフェクト
};