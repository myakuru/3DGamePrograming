#include "GameScene.h"
#include"../SceneManager.h"
#include"../../GameObject/Utility/Time.h"

void GameScene::Event()
{
	KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_brightThreshold);
	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(m_fogEnable, m_fogUseRange);
	KdShaderManager::Instance().WorkAmbientController().SetDistanceFog({ m_fogColor }, m_fogDensity);

	if (KeyboardManager::GetInstance().IsKeyPressed('P'))
	{
		KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(true); // ノイズON
		KdShaderManager::Instance().m_postProcessShader.SetNoiseStrength(0.07f);
	}
	else
	{
		KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(false); // ノイズON
	}

	Time::Instance().Update();

	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}

	Time::Instance().Update();

}

void GameScene::Init()
{
	// PostProcess用パラメータ
	m_brightThreshold = 0.9f;
	m_fogEnable = true;
	m_fogUseRange = false;
	m_fogColor = { 0.7f, 0.7f, 0.7f };
	m_fogDensity = 0.005f;

	SceneManager::Instance().SetDrawGrayScale(false);

	Time::Instance().StartTimer();

	SceneManager::Instance().m_gameClear = false; // ゲームクリアフラグを初期化
	SceneManager::Instance().SetResultFlag(false); // 結果フラグを初期化

}
