#pragma once
#include"../EffekseerEffectBase.h"
class AvoidAttackSlashEffect : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;
	AvoidAttackSlashEffect() { m_typeID = TypeID; }
	~AvoidAttackSlashEffect() override = default;
};