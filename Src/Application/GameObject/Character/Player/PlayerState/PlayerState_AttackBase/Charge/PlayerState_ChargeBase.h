#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/PlayerState_AttackBase.h"

class PlayerState_ChargeBase : public PlayerState_AttackBase
{
public:
	PlayerState_ChargeBase() = default;
	~PlayerState_ChargeBase() override = default;


	void StateUpdate(Player* _owner) override;

private:

	float m_overshootDist = 4.0f;

};