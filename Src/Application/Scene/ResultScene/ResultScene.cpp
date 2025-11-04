#include "ResultScene.h"
#include"../SceneManager.h"
#include"../../GameObject/Utility/Time.h"

void ResultScene::Event()
{
	KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_brightThreshold);
	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_anviLightColor);
	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(m_fogEnable, m_fogUseRange);
	KdShaderManager::Instance().WorkAmbientController().SetDistanceFog({ m_fogColor }, m_fogDensity);
	KdShaderManager::Instance().WorkAmbientController().SetheightFog({ 0.4f, 0.3f, 0.3f }, 100.0f, 0.0f, 0.0f);

	Time::Instance().Update();

	float time = Time::Instance().GetElapsedTime();
	int sec = static_cast<int>(time);

	if (sec == 0 || sec % 2 == 1)
	{
		// ランダムでノイズON/OFF
		bool enableNoise = KdRandom::GetInt(0, 5) == 1;
		KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(enableNoise);
		if (enableNoise)
		{
			float noiseStrength = KdRandom::GetFloat(0.01f, 0.2f);
			KdShaderManager::Instance().m_postProcessShader.SetNoiseStrength(noiseStrength);
		}
	}
	else
	{
		KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(false); // ノイズOFF
	}
}

void ResultScene::Init()
{
	KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
	SceneManager::Instance().SetDrawGrayScale(true);

	SceneManager::Instance().SetResultFlag(true);

	Time::Instance().StartTimer();

	KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_brightThreshold);
	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(m_fogEnable, m_fogUseRange);
	KdShaderManager::Instance().WorkAmbientController().SetDistanceFog({ m_fogColor }, m_fogDensity);
	KdShaderManager::Instance().WorkAmbientController().SetheightFog({ m_highFogColor }, m_highFogHeight, m_lowFogHeight, m_highFogDistance);

	KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_directionalLightDir, m_directionalLightColor);
	KdShaderManager::Instance().WorkAmbientController().SetDirLightShadowArea(m_lightingArea, m_dirLightHeight);

	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_anviLightColor);

	// PostProcess用パラメータ
	m_brightThreshold = 0.9f;
	m_fogEnable = true;
	m_fogUseRange = true;
	m_fogColor = { 0.3f, 0.3f, 0.3f };
	m_fogDensity = 0.005f;
}
