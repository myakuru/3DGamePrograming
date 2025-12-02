#include "Application/Scene/SceneManager.h" 
#include "../../../../Utility/Time.h"
#include "PlayerCameraState_IntroCamera.h"
#include "Application/main.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_LookPlayer/PlayerCameraState_LookPlayer.h"

void PlayerCameraState_IntroCamera::StateStart() 
{
	m_started = false;
	m_timer = 0.0f;

	// 状態データ参照
	auto& look = m_playerCamera->LookState();
	auto& intro = m_playerCamera->IntroState();
	const auto& cfg = m_playerCamera->IntroConfigRef();

	// プレイヤーの斜め前あたりからスタート
	m_playerCamera->SetPlayerRotation({ m_playerCamera->GetPlayerRotation().x, cfg.startYawDeg, m_playerCamera->GetPlayerRotation().z });
	intro.startYaw = m_playerCamera->GetPlayerRotation().y;
	intro.introTimer = 0.0f;
	look.followRate = cfg.startFollow;
	intro.inited = true;
	SceneManager::Instance().SetIntroCamera(true);
}

void PlayerCameraState_IntroCamera::StateUpdate()
{
	auto* cam = m_playerCamera; if (!cam) return;
	float deltaTime = Application::Instance().GetDeltaTime();

	// 状態データ参照
	auto& look = cam->LookState();
	auto& intro = cam->IntroState();
	const auto& cfg = cam->IntroConfigRef();

	// 回転進行
	cam->SetPlayerRotation({ cam->GetPlayerRotation().x,
		cam->GetPlayerRotation().y + cfg.yawSpeedDegPerSec * deltaTime,
		cam->GetPlayerRotation().z });

	// 徐々に近づける
	m_timer += deltaTime;
	look.followRate = Math::Vector3::SmoothStep(cfg.startFollow, cfg.endFollow, m_timer);

	// 回転終了判定
	if (cam->GetPlayerRotation().y >= cfg.endYawDeg)
	{
		cam->SetPlayerRotation({ cam->GetPlayerRotation().x, cfg.endYawDeg, cam->GetPlayerRotation().z });
		look.followRate = cfg.endFollow;

		intro.introTimer += deltaTime;

		// 回転が終わったら後ろに下がって、1秒してから通常カメラへ移行
		if (intro.introTimer >= cfg.holdSeconds)
		{
			const float afterT = std::clamp((intro.introTimer - cfg.holdSeconds) / 0.8f, 0.0f, 1.0f);
			float z = std::lerp(cfg.endFollow.z, cfg.afterEndFollowZ, afterT);
			look.followRate = { cfg.endFollow.x, cfg.endFollow.y, z };

			if (afterT >= 1.0f)
			{
				// 1秒経過したら通常カメラへ移行
				intro.inited = false;
				intro.introTimer = 0.0f;
				m_timer = 0.0f;
				SceneManager::Instance().SetIntroCamera(false);
				cam->ChangeState(std::make_shared<PlayerCameraState_LookPlayer>());
			}
		}
	}
}

void PlayerCameraState_IntroCamera::StateEnd()
{
	// 既存のフラグをOFF
	SceneManager::Instance().SetIntroCamera(false);
}
