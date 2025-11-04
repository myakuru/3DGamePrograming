#pragma once
#include"../EffekseerEffectBase.h"
class BossEnemy;
class BossWaterFallAttack : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	BossWaterFallAttack() { m_typeID = TypeID; }
	~BossWaterFallAttack() override = default;

private:

	void Update() override;

	std::list<std::weak_ptr<BossEnemy>> m_bossList;

	std::weak_ptr<BossEnemy> m_boss;

	Math::Vector3 m_bossPos = Math::Vector3::Zero;

};