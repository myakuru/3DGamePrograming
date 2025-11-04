#pragma once
#include"../EffekseerEffectBase.h"
class BossEnemy;
class BossAttack_1stEffect : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	BossAttack_1stEffect() { m_typeID = TypeID; }
	~BossAttack_1stEffect() override = default;

private:

	void Init() override;
	void Update() override;

	std::list<std::weak_ptr<BossEnemy>> m_bossList;

	std::weak_ptr<BossEnemy> m_boss;

	Math::Vector3 m_bossPos = Math::Vector3::Zero;

};