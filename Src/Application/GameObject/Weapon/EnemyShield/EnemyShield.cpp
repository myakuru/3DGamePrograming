#include "EnemyShield.h"

const uint32_t EnemyShield::TypeID = KdGameObject::GenerateTypeID();

void EnemyShield::Update()
{
	WeaponBase::Update();

	// オフセットなしで合成
	m_swordData.m_weaponMatrix = ComposeWeaponMatrix
	(
		m_enemyLeftSwordData,						// TRS
		m_enemyLeftSwordData,						// ボーン
		Math::Vector3::Zero,						// オフセット
		m_enemyRightSwordData.m_enemyWorldMatrix	// 親ワールド(これは右のデーター取ってくる)
	);
}
