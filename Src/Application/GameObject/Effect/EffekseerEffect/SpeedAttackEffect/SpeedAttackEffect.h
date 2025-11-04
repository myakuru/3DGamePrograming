#pragma once
#include"../EffekseerEffectBase.h"
class SpeedAttackEffect :public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;
	SpeedAttackEffect() { m_typeID = TypeID; }
	~SpeedAttackEffect() override = default;

};