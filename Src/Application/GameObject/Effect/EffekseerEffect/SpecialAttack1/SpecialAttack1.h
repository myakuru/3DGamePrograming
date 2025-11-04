#pragma once
#include"../EffekseerEffectBase.h"
class SpecialAttack1 : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;
	SpecialAttack1() { m_typeID = TypeID; }
	~SpecialAttack1() override = default;

};