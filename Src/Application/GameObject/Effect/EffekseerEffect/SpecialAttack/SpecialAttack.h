#pragma once
#include"../EffekseerEffectBase.h"
class SpecialAttack :public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	SpecialAttack() { m_typeID = TypeID; }
	~SpecialAttack() override = default;
};