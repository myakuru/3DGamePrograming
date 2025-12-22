#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyState_Dodge : public BossEnemyStateBase
{
public:
	BossEnemyState_Dodge() = default;
	~BossEnemyState_Dodge()override = default;

private:

	void StateStart(BossEnemy* _owner) override;
	void StateUpdate(BossEnemy* _owner) override;
	void StateEnd(BossEnemy* _owner) override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

	void ApplyFromConfig(const BossEnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

};