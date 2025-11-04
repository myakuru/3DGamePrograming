#pragma once
#include"../PlayerState.h"
class SpecialAttack1;
class SpecialAttackSmoke;
class PlayerState_SpecialAttack1 :public PlayerStateBase
{
public:

	PlayerState_SpecialAttack1() = default;
	~PlayerState_SpecialAttack1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	PlayerState_Attack2Parameter m_attackParam;

	std::weak_ptr<SpecialAttack1> m_effect;

	bool m_effectOnce = false;       // 再生要求を出したか
	bool m_effectStarted = false;    // 実際に再生が始まったか（IsPlaying が最初に true になった）

	float m_yawRad = 0.0f;

	float m_yawDeg = 0.0f;

	std::weak_ptr<SpecialAttackSmoke> m_smokeEffect;

	bool m_playSound = false;

};