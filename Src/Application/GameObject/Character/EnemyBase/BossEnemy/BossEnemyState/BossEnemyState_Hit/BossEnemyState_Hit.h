#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Hit : public BossEnemyStateBase
{
public:
	BossEnemyState_Hit() = default;
	~BossEnemyState_Hit()override = default;

private:

	void StateStart(BossEnemy* _owner) override;
	void StateUpdate(BossEnemy* _owner) override;
	void StateEnd(BossEnemy* _owner) override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	int m_hitCount = 0;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};