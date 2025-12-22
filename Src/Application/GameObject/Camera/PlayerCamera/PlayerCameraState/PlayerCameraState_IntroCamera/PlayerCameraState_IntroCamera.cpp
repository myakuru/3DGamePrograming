#include "Application/Scene/SceneManager.h" 
#include "../../../../Utility/Time.h"
#include "PlayerCameraState_IntroCamera.h"
#include "Application/main.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_LookPlayer/PlayerCameraState_LookPlayer.h"

void PlayerCameraState_IntroCamera::StateStart(PlayerCamera* _owner)
{
	m_started = false;
	m_timer = 0.0f;

	// 状態データ参照
	auto& look = _owner->LookState();
	auto& intro = _owner->IntroState();
	const auto& cfg = _owner->IntroConfigRef();

	// プレイヤーの斜め前あたりからスタート
	_owner->SetPlayerRotation({ _owner->GetPlayerRotation().x, cfg.startYawDeg, _owner->GetPlayerRotation().z });
	intro.startYaw = _owner->GetPlayerRotation().y;
	intro.introTimer = 0.0f;
	look.followRate = cfg.startFollow;
	intro.inited = true;
	SceneManager::Instance().SetIntroCamera(true);
}

void PlayerCameraState_IntroCamera::StateUpdate(PlayerCamera* _owner)
{
	auto* cam = _owner; if (!cam) return;
	float deltaTime = Application::Instance().GetDeltaTime();

	// 状態データ参照
	auto& look = _owner->LookState();
	auto& intro = _owner->IntroState();
	const auto& cfg = _owner->IntroConfigRef();

	// 回転進行
	_owner->SetPlayerRotation({ _owner->GetPlayerRotation().x,
		_owner->GetPlayerRotation().y + cfg.yawSpeedDegPerSec * deltaTime,
		_owner->GetPlayerRotation().z });

	// 徐々に近づける
	m_timer += deltaTime;
	look.followRate = Math::Vector3::SmoothStep(cfg.startFollow, cfg.endFollow, m_timer);

	// 回転終了判定
	if (_owner->GetPlayerRotation().y >= cfg.endYawDeg)
	{
		_owner->SetPlayerRotation({ _owner->GetPlayerRotation().x, cfg.endYawDeg, _owner->GetPlayerRotation().z });
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

void PlayerCameraState_IntroCamera::StateEnd(PlayerCamera* _owner)
{
	// 既存のフラグをOFF
	SceneManager::Instance().SetIntroCamera(false);
}
