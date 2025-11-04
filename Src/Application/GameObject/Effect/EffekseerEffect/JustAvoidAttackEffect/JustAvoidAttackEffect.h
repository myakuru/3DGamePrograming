#pragma once
#include"../EffekseerEffectBase.h"
class JustAvoidAttackEffect : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	JustAvoidAttackEffect() { m_typeID = TypeID; }
	~JustAvoidAttackEffect() override = default;
};