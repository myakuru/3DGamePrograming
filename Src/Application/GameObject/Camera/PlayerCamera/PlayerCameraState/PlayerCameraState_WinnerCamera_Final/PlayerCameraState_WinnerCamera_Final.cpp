#include "PlayerCameraState_WinnerCamera_Final.h"
#include "../../../../Utility/Time.h"
#include "Application/main.h"
#include "Application/Scene/SceneManager.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"

void PlayerCameraState_WinnerCamera_Final::StateStart()
{
	auto* cam = m_playerCamera; if (!cam) return;

	// Winner 演出開始
	auto& win = cam->WinnerState();
	win.time = 0.0f;

	// スローモーション（演出停止）
	Application::Instance().SetFpsScale(0.0f);
}

void PlayerCameraState_WinnerCamera_Final::StateUpdate()
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

		// 偶数秒の間だけノイズ有効
		if (sec == 0 || sec % 2 == 0)
		{
			// 5分の1の確率でノイズ有効
			m_enableNoiseRandom.x = 0.0f;	// KdRandomの都合上ｘは０に固定する。
			bool enableNoise = KdRandom::GetInt(static_cast<int>(m_enableNoiseRandom.x), static_cast<int>(m_enableNoiseRandom.y)) == 1;
			KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(enableNoise);

			// ノイズ強度ランダム設定
			if (enableNoise)
			{
				float noiseStrength = KdRandom::GetFloat(m_noiseStrength.x, m_noiseStrength.y);
				KdShaderManager::Instance().m_postProcessShader.SetNoiseStrength(noiseStrength);
			}
		}
		else
		{
			KdShaderManager::Instance().m_postProcessShader.SetEnableNoise(false);
		}
	}

	if (win.time > 0.0f)
	{
		cam->SetPlayerRotation(m_cameraRotation);
		look.followRate = Math::Vector3::SmoothStep(m_startFollow, m_endFollow, win.time / 2);

		KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);

		// 終了条件
		if (look.targetLookAt.z >= -0.1f)
		{
			SceneManager::Instance().m_gameClear = false;
			win.time = 0.0f;
			Application::Instance().SetFpsScale(1.0f);
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
		}
	}
}

void PlayerCameraState_WinnerCamera_Final::StateEnd()
{
}

void PlayerCameraState_WinnerCamera_Final::ApplyFromConfig(const PlayerCameraState& other)
{
	assert(typeid(other) == typeid(PlayerCameraState_WinnerCamera_Final));
	const auto& p = static_cast<const PlayerCameraState_WinnerCamera_Final&>(other);
	m_startFollow = p.m_startFollow;
	m_endFollow = p.m_endFollow;
	m_cameraRotation = p.m_cameraRotation;
	m_noiseStrength = p.m_noiseStrength;
	m_enableNoiseRandom = p.m_enableNoiseRandom;
}

void PlayerCameraState_WinnerCamera_Final::ExposeParametersImGui()
{
	ImGui::DragFloat3(U8("WinnerCamera Start Follow"), &m_startFollow.x, 0.01f);
	ImGui::DragFloat3(U8("WinnerCamera End Follow"), &m_endFollow.x, 0.01f);
	ImGui::DragFloat3(U8("Camera Rotation"), &m_cameraRotation.x, 0.1f);
	ImGui::DragFloat2(U8("Noise Strength Min/Max"), &m_noiseStrength.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat2(U8("Enable Noise Random Min/Max"), &m_enableNoiseRandom.x, 1.0f, 0.0f, 20.0f);
}

void PlayerCameraState_WinnerCamera_Final::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerCameraState_WinnerCamera_Final")) return;
	const auto& stateNode = js["PlayerCameraState_WinnerCamera_Final"];

	if (stateNode.contains("startFollow")) m_startFollow = JSON_MANAGER.JsonToVector(stateNode["startFollow"]);
	if (stateNode.contains("endFollow"))   m_endFollow = JSON_MANAGER.JsonToVector(stateNode["endFollow"]);
	if (stateNode.contains("cameraRotation"))   m_cameraRotation = JSON_MANAGER.JsonToVector(stateNode["cameraRotation"]);
	if (stateNode.contains("noiseStrength"))   m_noiseStrength = JSON_MANAGER.JsonToVector2(stateNode["noiseStrength"]);
	if (stateNode.contains("enableNoiseRandom"))   m_enableNoiseRandom = JSON_MANAGER.JsonToVector2(stateNode["enableNoiseRandom"]);
}

void PlayerCameraState_WinnerCamera_Final::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("PlayerCameraState_WinnerCamera_Final")) js["PlayerCameraState_WinnerCamera_Final"] = nlohmann::json::object();
	auto& stateNode = js["PlayerCameraState_WinnerCamera_Final"];

	stateNode["startFollow"] = JSON_MANAGER.VectorToJson(m_startFollow);
	stateNode["endFollow"] = JSON_MANAGER.VectorToJson(m_endFollow);
	stateNode["cameraRotation"] = JSON_MANAGER.VectorToJson(m_cameraRotation);
	stateNode["noiseStrength"] = JSON_MANAGER.Vector2ToJson(m_noiseStrength);
	stateNode["enableNoiseRandom"] = JSON_MANAGER.Vector2ToJson(m_enableNoiseRandom);
}
