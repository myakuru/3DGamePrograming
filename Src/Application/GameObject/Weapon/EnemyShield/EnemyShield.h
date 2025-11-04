#pragma once
#include"../WeaponBase.h"
class EnemyShield : public WeaponBase
{
public:

	static const uint32_t TypeID;

	EnemyShield() { m_typeID = TypeID; AddTag(ObjTag::EnemyShield); }
	~EnemyShield() override = default;

private:

	void Update() override;

};