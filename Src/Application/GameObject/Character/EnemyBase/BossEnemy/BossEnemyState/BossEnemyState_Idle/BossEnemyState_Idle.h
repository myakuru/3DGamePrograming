#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Idle : public BossEnemyStateBase
{
public:
	BossEnemyState_Idle() = default;
	explicit BossEnemyState_Idle(float minWaitSec) : m_minWaitSec(minWaitSec) {}
	~BossEnemyState_Idle()override = default;

private:
	void StateStart(BossEnemy* _owner) override;
	void StateUpdate(BossEnemy* _owner) override;
	void StateEnd(BossEnemy* _owner) override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

	float m_minWaitSec = 0.0f; // 最低待機秒
};