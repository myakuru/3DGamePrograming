#include "EnemySword.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/ImGuiManager/ImGuiManager.h"

const uint32_t EnemySword::TypeID = KdGameObject::GenerateTypeID();

void EnemySword::Update()
{
	// オフセットなしで合成
	m_swordData.m_weaponMatrix = ComposeWeaponMatrix(
		m_enemyRightSwordData,         // TRS(角度・スケール)
		m_enemyRightSwordData,         // ボーン
		Math::Vector3::Zero,           // オフセット
		m_enemyRightSwordData.m_enemyWorldMatrix); // 親ワールド
}