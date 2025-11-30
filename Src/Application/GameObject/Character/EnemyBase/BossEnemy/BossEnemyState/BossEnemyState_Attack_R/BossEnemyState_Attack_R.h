#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossAttack_1stEffect;

class BossEnemyState_Attack_R : public BossEnemyStateBase
{
public:
	BossEnemyState_Attack_R() = default;
	~BossEnemyState_Attack_R()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

	std::weak_ptr<BossAttack_1stEffect> m_attackEffect;

};