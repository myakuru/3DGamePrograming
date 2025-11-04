#pragma once
#include"../PlayerState.h"
class AttacEffect1;
class PlayerState_Attack2 : public PlayerStateBase
{
public:
	PlayerState_Attack2() = default;
	~PlayerState_Attack2() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	PlayerState_AttackParameter m_attackParam;

	std::weak_ptr<AttacEffect1> m_slashEffect;


	bool m_flag = false; // 攻撃フラグ

};