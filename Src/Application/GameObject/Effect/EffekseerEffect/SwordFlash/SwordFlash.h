#pragma once
#include"../EffekseerEffectBase.h"
class SwordFlash : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	SwordFlash() { m_typeID = TypeID; }
	~SwordFlash() override = default;

};