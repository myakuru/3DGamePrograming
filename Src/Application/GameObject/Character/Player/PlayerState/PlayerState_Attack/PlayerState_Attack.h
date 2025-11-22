#pragma once
#include"../PlayerState.h"
class TrailEffect;
class SwordFlash;
class PlayerState_Attack :public PlayerStateBase
{
public:
	PlayerState_Attack() = default;
	~PlayerState_Attack() override = default;

	void ApplyFromConfig(const PlayerStateBase& other) override
	{
		// デバッグ時に型が一致することを確認（リリースでは削除）
		assert(typeid(other) == typeid(PlayerState_Attack));

		// 呼び出し元が同型を保証している前提で安全に static_cast
		const auto& p = static_cast<const PlayerState_Attack&>(other);

		m_attackRadius = p.m_attackRadius;
		m_attackDistance = p.m_attackDistance;
		m_attackCount = p.m_attackCount;
		m_attackTime = p.m_attackTime;
		m_attackStartTime = p.m_attackStartTime;
		m_attackEndTime = p.m_attackEndTime;
		m_moveSpeed = p.m_moveSpeed;
	}

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void ExposeParametersImGui() override;

	void LoadParametersJson(const nlohmann::json& _json) override;

	void SaveParametersJson(nlohmann::json& _json) const override;

	std::weak_ptr<TrailEffect> m_trailEffect;
	std::weak_ptr<SwordFlash> m_slashEffect;

	float m_attackRadius = 1.0f;
	float m_attackDistance = 1.0f;
	int  m_attackCount = 1;
	float m_attackTime = 0.1f;
	Math::Vector2 m_cameraShakeParam = { 0.2f,0.0f };
	float m_cameraTime = 0.3f;
	float m_attackStartTime = 0.0f;
	float m_attackEndTime = 0.4f;
	Math::Vector3 m_moveSpeed = Math::Vector3::Zero;

};