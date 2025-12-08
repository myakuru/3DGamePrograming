#pragma once
#include"../EnemyStateBase.h"
class EnemyState_Attack1 : public EnemyStateBase
{
public:
	EnemyState_Attack1() = default;
	~EnemyState_Attack1()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ApplyFromConfig(const EnemyStateBase& other) override;
	void ExposeParametersImGui() override;
	void LoadParametersJson(const nlohmann::json& js) override;
	void SaveParametersJson(nlohmann::json& js) const override;

	float m_hitStopTimer = 0.0f;				// ヒットストップでの経過時間
	bool m_stopped = false;						// アニメーション停止フラグ
	float m_animationStopTime = 0.2f;			// アニメーション停止時間
	float m_hitStopTime = 0.1f;					// ヒットストップ時間
	const float m_KdefaultHitStopTime = 1.0f;	// デフォルトヒットストップ時間
};