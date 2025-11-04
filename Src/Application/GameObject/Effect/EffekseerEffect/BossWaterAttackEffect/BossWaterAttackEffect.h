#pragma once
#include"../EffekseerEffectBase.h"
class BossEnemy;
class BossWaterAttackEffect : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	BossWaterAttackEffect() { m_typeID = TypeID; }
	~BossWaterAttackEffect() override = default;

private:

	void Update() override;

	std::list<std::weak_ptr<BossEnemy>> m_bossList;

	std::weak_ptr<BossEnemy> m_boss;

	Math::Vector3 m_bossPos = Math::Vector3::Zero;

};