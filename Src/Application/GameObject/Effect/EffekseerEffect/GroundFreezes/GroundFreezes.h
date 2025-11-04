#pragma once
#include"../EffekseerEffectBase.h"
class GroundFreezes : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	GroundFreezes() { m_typeID = TypeID; }
	~GroundFreezes() override = default;
};