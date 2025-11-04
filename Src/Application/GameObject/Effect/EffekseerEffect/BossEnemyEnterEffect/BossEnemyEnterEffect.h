#pragma once
#include"../EffekseerEffectBase.h"
class BossEnemy;
class BossEnemyEnterEffect : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	BossEnemyEnterEffect() { m_typeID = TypeID; }
	~BossEnemyEnterEffect() override = default;

private:

	void Init() override;
	void Update() override;

	std::list<std::weak_ptr<BossEnemy>> m_bossList;

	std::weak_ptr<BossEnemy> m_boss;

	Math::Vector3 m_bossPos = Math::Vector3::Zero;

};