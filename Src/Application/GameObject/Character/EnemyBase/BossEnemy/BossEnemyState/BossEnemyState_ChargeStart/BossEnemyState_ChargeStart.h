#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_ChargeStart : public BossEnemyStateBase
{
public:
	BossEnemyState_ChargeStart() = default;
	~BossEnemyState_ChargeStart()override = default;

private:

	void StateStart(BossEnemy* _owner) override;
	void StateUpdate(BossEnemy* _owner) override;
	void StateEnd(BossEnemy* _owner) override;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};