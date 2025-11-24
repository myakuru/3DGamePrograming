#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Hit : public BossEnemyStateBase
{
public:
	BossEnemyState_Hit() = default;
	~BossEnemyState_Hit()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	int m_hitCount = 0;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};