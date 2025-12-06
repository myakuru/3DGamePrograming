#pragma once
#include"Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"

class EnemyEffect : public EffekseerEffectBase
{
public:

	static const uint32_t TypeID;

	EnemyEffect() { m_typeID = TypeID; }
	~EnemyEffect() override = default;

private:

	void Update() override;
	void EffectUpdate() override;

};