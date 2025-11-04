#pragma once
#include"../EffekseerEffectBase.h"
class AttacEffect1 : public EffekseerEffectBase
{
public:
	static const uint32_t TypeID;

	AttacEffect1() { m_typeID = TypeID; }
	~AttacEffect1() override = default;

};