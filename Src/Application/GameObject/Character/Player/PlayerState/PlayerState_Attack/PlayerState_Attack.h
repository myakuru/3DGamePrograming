#pragma once
#include"../PlayerState.h"
class TrailEffect;
class SwordFlash;
class PlayerState_Attack :public PlayerStateBase
{
public:
	PlayerState_Attack() = default;
	~PlayerState_Attack() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	PlayerState_Attack2Parameter m_attackParam;

	std::weak_ptr<TrailEffect> m_trailEffect;
	std::weak_ptr<SwordFlash> m_slashEffect;

};