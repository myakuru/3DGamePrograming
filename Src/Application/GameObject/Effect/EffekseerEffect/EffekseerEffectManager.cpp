#include "EffekseerEffectManager.h"
#include "AttacEffect1/AttacEffect1.h"

void EffekseerEffectManager::Init()
{
	m_attacEffect1 = std::make_shared<AttacEffect1>();
}