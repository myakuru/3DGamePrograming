#pragma once
#include"../PlayerState.h"
class SlashAttack2Effect;
class PlayerState_Attack3 :public PlayerStateBase
{
public:
	PlayerState_Attack3() = default;
	~PlayerState_Attack3() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::weak_ptr<SlashAttack2Effect> m_slashEffect;

	PlayerState_AttackParameter m_attackParam;

};