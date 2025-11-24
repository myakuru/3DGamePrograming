#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyEnterEffect;
class BossEnemyState_Enter : public BossEnemyStateBase
{
public:
	BossEnemyState_Enter() = default;
	~BossEnemyState_Enter()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;
	std::weak_ptr<BossEnemyEnterEffect> m_enterEffect;

	bool m_effectPlayed = false;

};