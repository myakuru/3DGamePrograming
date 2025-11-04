#pragma once
#include"../EffekseerEffectBase.h"
class EffectPlay : public EffekseerEffectBase
{
public:
	static const uint32_t TypeID;

	EffectPlay() { m_typeID = TypeID; }
	~EffectPlay() override =  default;

};