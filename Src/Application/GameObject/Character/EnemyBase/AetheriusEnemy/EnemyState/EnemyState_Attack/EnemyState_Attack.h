#pragma once
#include"../EnemyStateBase.h"
class EnemyShineBlue;
class AetheriusEnemy_AttackEffect;

class EnemyState_Attack : public EnemyStateBase
{
public:
	EnemyState_Attack() = default;
	~EnemyState_Attack()override = default;

private:
	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;
	std::weak_ptr<AetheriusEnemy_AttackEffect> m_attackEffect;

};