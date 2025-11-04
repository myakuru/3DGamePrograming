#pragma once
#include"../PlayerState.h"
class GroundFreezes;
class Rotation;
class PlayerState_Attack4 :public PlayerStateBase
{
public:
	PlayerState_Attack4() = default;
	~PlayerState_Attack4() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::weak_ptr<GroundFreezes> m_groundFreezes;
	std::weak_ptr<Rotation> m_rotation;

	PlayerState_AttackParameter m_attackParam;
};