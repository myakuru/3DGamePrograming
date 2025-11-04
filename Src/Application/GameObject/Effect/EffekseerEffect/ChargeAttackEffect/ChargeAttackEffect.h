#pragma once
#include"../EffekseerEffectBase.h"
class ChargeAttackEffect : public EffekseerEffectBase
{
public:
	ChargeAttackEffect() = default;
	~ChargeAttackEffect() override = default;

	static const uint32_t TypeID;
};