#pragma once
#include"../EffekseerEffectBase.h"
class SpecialAttackSmoke : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;
	SpecialAttackSmoke() { m_typeID = TypeID; }
	~SpecialAttackSmoke() override = default;

private:

	void DrawLit() override;

};