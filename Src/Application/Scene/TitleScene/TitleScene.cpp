#include "TitleScene.h"
#include "../SceneManager.h"

#include"../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../main.h"

void TitleScene::Event()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Test
		);
	}
}

void TitleScene::Init()
{
	auto& sceneManager = SceneManager::Instance();

	sceneManager.SetIntroCamera(false);

	sceneManager.SetDrawGrayScale(false);
	sceneManager.SetResultFlag   (false);	// 結果フラグを初期化
	sceneManager.m_gameClear = false;	    // ゲームクリアフラグを初期化
}
