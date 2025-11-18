#pragma once
#include "EffekseerEffectBase.h"
class EffekseerEffectManager : public EffekseerEffectBase
{
public:
	EffekseerEffectManager() = default;
	~EffekseerEffectManager() override = default;

private:

	std::list<std::shared_ptr<EffekseerEffectBase>> m_effectObjectList;

};