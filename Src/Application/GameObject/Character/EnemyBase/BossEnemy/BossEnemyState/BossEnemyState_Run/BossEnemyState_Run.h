#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Run : public BossEnemyStateBase
{
public:
	BossEnemyState_Run() = default;
	~BossEnemyState_Run()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	float m_targetDistance = 10.0f;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};