#pragma once
#include"../BossEnemyState.h"
class EnemyShineBlue;
class BossEnemyEnterEffect;
class BossEnemyState_Enter : public BossEnemyStateBase
{
public:
	BossEnemyState_Enter() = default;
	~BossEnemyState_Enter()override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	std::list<std::weak_ptr<EnemyShineBlue>> m_shineEffectBlues;
	std::weak_ptr<BossEnemyEnterEffect> m_enterEffect;

	bool m_effectPlayed = false;

};