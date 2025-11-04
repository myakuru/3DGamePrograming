#pragma once
#include"../EffekseerEffectBase.h"
class JustAvoidAttackEffect1 : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	JustAvoidAttackEffect1() { m_typeID = TypeID; }
	~JustAvoidAttackEffect1() override = default;
};