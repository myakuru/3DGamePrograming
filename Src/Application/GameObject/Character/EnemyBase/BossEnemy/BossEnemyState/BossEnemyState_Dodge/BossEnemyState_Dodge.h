#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Dodge : public BossEnemyStateBase
{
public:
	BossEnemyState_Dodge() = default;
	~BossEnemyState_Dodge()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};