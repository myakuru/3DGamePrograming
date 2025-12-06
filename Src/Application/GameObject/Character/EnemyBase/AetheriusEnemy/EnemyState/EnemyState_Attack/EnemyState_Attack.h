#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Attack : public EnemyStateBase
{
public:
	EnemyState_Attack() = default;
	~EnemyState_Attack()override = default;

private:
	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	float m_fpsScale = 0.4f;

	float m_hitStopTimer = 0.0f;
	bool m_stopped = false;
	float m_animationStopTime = 0.5f;
	float m_hitStopTime = 0.1f;

	const float m_KdefaultHitStopTime = 1.0f;
	
};