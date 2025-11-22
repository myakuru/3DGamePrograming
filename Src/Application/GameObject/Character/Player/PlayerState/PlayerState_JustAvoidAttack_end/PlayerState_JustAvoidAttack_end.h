#pragma once
#include"../PlayerState.h"
class JustAvoidAttackEffect;
class PlayerState_JustAvoidAttack_end :public PlayerStateBase
{
public:
	PlayerState_JustAvoidAttack_end() = default;
	~PlayerState_JustAvoidAttack_end() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	Math::Vector3 m_targetPoint = Math::Vector3::Zero;
	bool m_reachedBehind = false;

	std::weak_ptr<JustAvoidAttackEffect> m_justAvoidAttackEffect;

};