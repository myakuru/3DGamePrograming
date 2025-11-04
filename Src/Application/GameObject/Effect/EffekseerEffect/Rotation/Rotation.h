#pragma once
#include"../EffekseerEffectBase.h"
class Rotation : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;
	Rotation() { m_typeID = TypeID; }
	~Rotation() override = default;
};