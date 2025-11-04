#pragma once
#include"../PlayerState.h"
class SpecialAttack;
class PlayerState_SpecialAttack :public PlayerStateBase
{
public:
	PlayerState_SpecialAttack() = default;
	~PlayerState_SpecialAttack() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::weak_ptr<SpecialAttack> m_specialAttackEffect;

	float m_yawRad = 0.0f;

	float m_yawDeg = 0.0f;

	bool m_playSound = false;

};