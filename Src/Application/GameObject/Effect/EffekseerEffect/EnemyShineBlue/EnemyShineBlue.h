#pragma once
#include"../EffekseerEffectBase.h"
class EnemyShineBlue : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;
	EnemyShineBlue() { m_typeID = TypeID; }
	~EnemyShineBlue() override = default;

private:

	void Update() override;

};