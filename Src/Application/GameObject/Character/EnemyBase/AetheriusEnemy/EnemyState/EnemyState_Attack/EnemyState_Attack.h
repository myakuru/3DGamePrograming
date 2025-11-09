#pragma once
#include"../EnemyStateBase.h"
class EnemyShineBlue;
class EnemyState_Attack : public EnemyStateBase
{
public:
	EnemyState_Attack() = default;
	~EnemyState_Attack()override = default;

private:
	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;

};