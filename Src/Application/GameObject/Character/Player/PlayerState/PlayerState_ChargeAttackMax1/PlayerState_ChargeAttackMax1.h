#pragma once
class ChargeAttackEffect;
#include"../PlayerState.h"
class PlayerState_ChargeAttackMax1 :public PlayerStateBase
{
public:
	PlayerState_ChargeAttackMax1() = default;
	~PlayerState_ChargeAttackMax1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::weak_ptr<ChargeAttackEffect> m_effect;

};