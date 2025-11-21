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
			SceneManager::SceneType::ConstructionSiteStage
		);
	}

	KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_brightThreshold);
	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(m_fogEnable, m_fogUseRange);
	KdShaderManager::Instance().WorkAmbientController().SetDistanceFog({ m_fogColor }, m_fogDensity);
	KdShaderManager::Instance().WorkAmbientController().SetheightFog({ m_highFogColor }, m_highFogHeight, m_lowFogHeight, m_highFogDistance);

	KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_directionalLightDir, m_directionalLightColor);
	KdShaderManager::Instance().WorkAmbientController().SetDirLightShadowArea(m_lightingArea, m_dirLightHeight);

	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_anviLightColor);
}

void TitleScene::Init()
{
	auto& sceneManager = SceneManager::Instance();

	// イントロBGM（非ループ）: SetGameSound を使う
	{
		auto intro = KdAudioManager::Instance().Play(
			"Asset/Sound/FieldBGM/ToDo_game_bgm.wav",
			true
		);
		SceneManager::Instance().SetGameSound(intro);

		if (intro)
		{
			intro->SetVolume(1.0f);
		}
	}

	sceneManager.SetIntroCamera(false);
	sceneManager.SetDrawGrayScale(false);
	sceneManager.SetResultFlag   (false);	// 結果フラグを初期化
	sceneManager.m_gameClear = false;		// ゲームクリアフラグを初期化
}
