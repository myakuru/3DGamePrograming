#pragma once
#include"../BossEnemyState.h"
class BossWaterFallAttack;
class BossEnemyState_WaterFall : public BossEnemyStateBase
{
public:
	BossEnemyState_WaterFall() = default;
	~BossEnemyState_WaterFall()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::weak_ptr<BossWaterFallAttack> m_waterFallAttack;

	bool m_oneSound = false;

};