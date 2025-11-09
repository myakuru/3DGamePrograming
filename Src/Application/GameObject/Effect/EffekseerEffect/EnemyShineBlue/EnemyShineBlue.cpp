#include"EnemyShineBlue.h"
#include"../../../../Scene/SceneManager.h"

const uint32_t EnemyShineBlue::TypeID = KdGameObject::GenerateTypeID();

void EnemyShineBlue::Update()
{
	KdEffekseerManager::GetInstance().Update();

	EffectUpdate();
}
