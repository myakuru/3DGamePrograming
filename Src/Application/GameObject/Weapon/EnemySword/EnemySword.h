#pragma once
#include"../WeaponBase.h"
class EnemySword : public WeaponBase
{
public:

	static const uint32_t TypeID;

	EnemySword() { m_typeID = TypeID; AddTag(ObjTag::EnemySword); }
	~EnemySword() override = default;

private:
	
	void Update() override;
};