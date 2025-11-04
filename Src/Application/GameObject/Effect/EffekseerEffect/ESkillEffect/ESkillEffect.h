#pragma once
#include"../EffekseerEffectBase.h"
class ESkillEffect : public EffekseerEffectBase
{
public:
	ESkillEffect () = default;
	~ESkillEffect () override = default;

	static const uint32_t TypeID;
};