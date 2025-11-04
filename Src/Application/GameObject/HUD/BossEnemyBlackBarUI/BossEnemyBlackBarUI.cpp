#include "BossEnemyBlackBarUI.h"
#include"../../../Scene/SceneManager.h"

const uint32_t BossEnemyBlackBarUI::TypeID = KdGameObject::GenerateTypeID();

void BossEnemyBlackBarUI::Update()
{
	// ボス敵が出現していなければ非表示
	if (!SceneManager::Instance().IsBossAppear())
	{
		m_bDrawTexture = false;
		return;
	}
	// 表示
	m_bDrawTexture = true;
}
