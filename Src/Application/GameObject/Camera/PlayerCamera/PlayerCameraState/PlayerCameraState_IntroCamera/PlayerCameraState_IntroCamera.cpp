#include "Application/Scene/SceneManager.h" 
#include "../../../../Utility/Time.h"
#include "PlayerCameraState_IntroCamera.h"
#include "Application/main.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_LookPlayer/PlayerCameraState_LookPlayer.h"

void PlayerCameraState_IntroCamera::StateStart() 
{
	m_started = false;
	// イントロ開始フラグを明示的にONにしておく
	SceneManager::Instance().SetIntroCamera(true);
}

void PlayerCameraState_IntroCamera::StateUpdate()
{
	auto* cam = m_playerCamera; if (!cam) return;
	float deltaTime = Application::Instance().GetDeltaTime();

	auto& look = cam->LookState();
	auto& intro = cam->IntroState();
	const auto& cfg = cam->IntroConfigRef();

	// 初回初期化
	if (!intro.inited)
	{
		cam->SetTargetRotation({ cam->GetTargetRotation().x, cfg.startYawDeg, cam->GetTargetRotation().z });
		intro.startYaw = cam->GetTargetRotation().y;
		intro.introTimer = 0.0f;
		look.followRate = cfg.startFollow;
		intro.inited = true;
		SceneManager::Instance().SetIntroCamera(true);
	}

	// 回転進行（マウス入力で乱されない前提）
	cam->SetTargetRotation({ cam->GetTargetRotation().x,
		cam->GetTargetRotation().y + cfg.yawSpeedDegPerSec * deltaTime,
		cam->GetTargetRotation().z });

	float denom = std::max(1e-4f, cfg.endYawDeg - intro.startYaw);
	float t = std::clamp((cam->GetTargetRotation().y - intro.startYaw) / denom, 0.0f, 1.0f);

	look.followRate = Math::Vector3::SmoothStep(cfg.startFollow, cfg.endFollow, t);

	if (cam->GetTargetRotation().y >= cfg.endYawDeg)
	{
		cam->SetTargetRotation({ cam->GetTargetRotation().x, cfg.endYawDeg, cam->GetTargetRotation().z });
		look.followRate = cfg.endFollow;

		intro.introTimer += deltaTime;

		// 終了待機後の余韻ズームアウトを補間
		if (intro.introTimer >= cfg.holdSeconds)
		{
			const float afterT = std::clamp((intro.introTimer - cfg.holdSeconds) / 0.8f, 0.0f, 1.0f);
			float z = std::lerp(cfg.endFollow.z, cfg.afterEndFollowZ, afterT);
			look.followRate = { cfg.endFollow.x, cfg.endFollow.y, z };

			if (afterT >= 1.0f)
			{
				intro.inited = false;
				intro.introTimer = 0.0f;
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
