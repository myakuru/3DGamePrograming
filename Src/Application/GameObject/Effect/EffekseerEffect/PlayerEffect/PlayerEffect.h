#pragma once
#include"Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
class PlayerEffect : public EffekseerEffectBase
{
public:
	
	static const uint32_t TypeID;

	PlayerEffect() { m_typeID = TypeID; }
	~PlayerEffect() override = default;



private:

	void Update() override;

};