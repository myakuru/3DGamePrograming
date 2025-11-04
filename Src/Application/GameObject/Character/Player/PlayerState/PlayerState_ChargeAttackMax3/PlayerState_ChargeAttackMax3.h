#pragma once
#include"../PlayerState.h"
class ChargeAttackEffect_end;
class PlayerState_ChargeAttackMax3 :public PlayerStateBase
{
public:
	PlayerState_ChargeAttackMax3() = default;
	~PlayerState_ChargeAttackMax3() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::weak_ptr<ChargeAttackEffect_end> m_effect;

};