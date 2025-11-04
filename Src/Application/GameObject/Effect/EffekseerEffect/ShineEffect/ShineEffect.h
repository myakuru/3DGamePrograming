#pragma once
#include"../EffekseerEffectBase.h"
class ShineEffect :public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	ShineEffect() { m_typeID = TypeID; }
	~ShineEffect() override = default;

};