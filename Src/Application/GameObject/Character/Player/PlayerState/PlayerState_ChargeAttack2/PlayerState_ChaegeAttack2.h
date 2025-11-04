#pragma once
#include"../PlayerState.h"
class EffectPlay;
class SmokeEffect;
class ShineEffect;
class PlayerState_ChaegeAttack2 :public PlayerStateBase
{
public:
	PlayerState_ChaegeAttack2() = default;
	~PlayerState_ChaegeAttack2() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::weak_ptr<EffectPlay> m_effect;
	std::weak_ptr<SmokeEffect> m_smokeEffect;
	std::weak_ptr<ShineEffect> m_shineEffect;

	float m_chargeAttackTimer = 0.0f; // チャージ攻撃のタイマー

	bool m_flag = false; // 攻撃フラグ
};