#include "Application/Scene/SceneManager.h"
#include "../../../../Utility/Time.h"
#include "PlayerCameraState_WinnerCamera.h"
#include "Application/main.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_LookPlayer/PlayerCameraState_LookPlayer.h"

void PlayerCameraState_WinnerCamera::StateStart()
{
	auto* cam = m_playerCamera; if (!cam) return;

	// Winner 演出開始
	auto& win = cam->WinnerState();
	win.time = 0.0f;

	// スローモーション（演出停止）
	Application::Instance().SetFpsScale(0.0f);

	// 初期ポストプロセス設定
	KdShaderManager::Instance().m_postProcessShader.SetEnableStrongBlur(false);
	KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(false);
	KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
}

void PlayerCameraState_WinnerCamera::StateUpdate()
{
	auto* cam = m_playerCamera; if (!cam) return;

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	auto& win = cam->WinnerState();
	auto& look = cam->LookState();

	// 演出経過時間
	win.time += deltaTime;

	// ノイズ点滅
	{
		float timeNow = Time::Instance().GetElapsedTime();
		int sec = static_cast<int>(timeNow);

		if (sec == 0 || sec % 2 == 1)
		{
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
			KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(false);
		}
	}

	// 区間[0,1)
	if (win.time >= 0.0f)
	{
		cam->SetTargetRotation({ -20.0f, 263.0f, 1.0f }); // m_degree = { -20,263,1 }
		const Math::Vector3 startFollow = { 0.0f, 0.7f, -1.0f };
		const Math::Vector3 endFollow = { 0.0f, 0.7f, -0.9f };
		float t = std::clamp(win.time, 0.0f, 1.0f);
		look.followRate = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		if (win.time >= 0.9f && win.time <= 0.99f)
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);
		else
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
	}

	// 区間[1,2)
	if (win.time >= 1.0f)
	{
		cam->SetTargetRotation({ 1.5f, 227.0f, 0.0f });
		const Math::Vector3 startFollow = { 3.4f, 1.0f, -4.4f };
		const Math::Vector3 endFollow = { 3.4f, 1.0f, -4.3f };
		float t = std::clamp(win.time - 1.0f, 0.0f, 1.0f);
		look.targetLookAt = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		if (win.time >= 1.9f && win.time <= 1.99f)
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);
		else
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
	}

	// 区間[2,3)
	if (win.time >= 2.0f)
	{
		cam->SetTargetRotation({ 10.0f, 320.0f, 0.0f });
		const Math::Vector3 startFollow = { 0.6f, 1.0f, -1.6f };
		const Math::Vector3 endFollow = { 0.6f, 1.0f, -1.5f };
		float t = std::clamp(win.time - 2.0f, 0.0f, 1.0f);
		look.targetLookAt = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		if (win.time >= 2.9f && win.time <= 2.99f)
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);
		else
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
	}

	// 区間[3, ∞)
	if (win.time >= 3.0f)
	{
		cam->SetTargetRotation({ 0.0f, 90.0f, 0.0f });
		const Math::Vector3 startFollow = { 0.0f, 0.7f, -4.0f };
		const Math::Vector3 endFollow = { 0.0f, 0.7f,  0.0f };
		float t = std::clamp(win.time - 3.0f, 0.0f, 1.0f);
		look.targetLookAt = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);

		// 終了条件
		if (look.targetLookAt.z >= 0.0f)
		{
			SceneManager::Instance().m_gameClear = false;
			win.time = 0.0f;
			Application::Instance().SetFpsScale(1.0f);
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
		}
	}
}

void PlayerCameraState_WinnerCamera::StateEnd()
{
	// 演出終了後にポストプロセス解除（必要なら）
	KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(false);
	KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
	// FPSを元に戻す保険（StateUpdate 終了条件でも戻している）
	Application::Instance().SetFpsScale(1.0f);
}