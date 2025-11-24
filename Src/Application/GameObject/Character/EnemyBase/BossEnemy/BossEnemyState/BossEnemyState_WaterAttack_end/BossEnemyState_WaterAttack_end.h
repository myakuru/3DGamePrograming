#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_WaterAttack_end : public BossEnemyStateBase
{
public:
	BossEnemyState_WaterAttack_end() = default;
	~BossEnemyState_WaterAttack_end()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};