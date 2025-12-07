#pragma once
#include"Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
class CharacterEffect : public EffekseerEffectBase
{
public:
	
	static const uint32_t TypeID;

	CharacterEffect() { m_typeID = TypeID; }
	~CharacterEffect() override = default;



private:

	void Update() override;

};