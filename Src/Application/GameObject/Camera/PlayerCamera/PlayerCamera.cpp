#include"../../../main.h"
#include"../../../Scene/SceneManager.h"
#include"../../Character/Player/Player.h"
#include"MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../Utility/Time.h"
#include"PlayerCameraState/PlayerCameraState.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_IntroCamera/PlayerCameraState_IntroCamera.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_WinnerCamera/PlayerCameraState_WinnerCamera.h"

const uint32_t PlayerCamera::TypeID = KdGameObject::GenerateTypeID();

void PlayerCamera::Init()
{
	CameraBase::Init();

	// エフェクシアのカメラをセット
	KdEffekseerManager::GetInstance().SetCamera(m_spCamera);

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	// 起動時にカーソルを動かさない。現在位置を保持
	GetCursorPos(&m_FixMousePos);

	// 状態参照
	auto& look = LookState();
	auto& fov = FovState();
	auto& col = CollisionState();
	auto& intro = IntroState();
	auto& shake = ShakeState();

	// 初期のカメラターゲット座標
	look.followRate = { 0.0f, 1.0f, -0.5f };

	fov.fovShake = { fov.fov, 0.0f };
	fov.fovShakeTarget = fov.fovShake;

	m_spCamera->SetProjectionMatrix(fov.fovShakeTarget.x);

	col.effectiveLookAt = look.targetLookAt;

	StateInit();

	SceneManager::Instance().GetObjectWeakPtr(m_Player);
	if (auto player = m_Player.lock(); player)
	{
		look.cameraPos = player->GetPos();
	}

	m_degree = { 0.0f, 140.0f, 0.0f };

	// IntroState 初期化
	intro.startYaw = 0.0f;
	intro.inited = false;
	intro.introTimer = 0.0f;
	shake.shakeTime = 0.0f;

	m_oneceFlag = false;
}

void PlayerCamera::PostUpdate()
{
	if (SceneManager::Instance().m_sceneCamera) return;

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	m_spTarget = m_Player.lock();
	if (!m_spTarget) return;

	// イントロ中はマウス回転を止める
	{
		bool introActive = SceneManager::Instance().IsIntroCamera();
		if (!introActive)
		{
			UpdateRotateByMouse();
		}
	}

	// ステート更新（各ステートがlook/fov/m_degree等を更新する）
	m_stateManager.Update();

	if (SceneManager::Instance().m_gameClear && !m_oneceFlag)
	{
		m_oneceFlag = true;

		auto winnerState = std::make_shared<PlayerCameraState_WinnerCamera>();
		ChangeState(winnerState);
		return;
	}

	// 以降は共通の行列更新・補間・衝突補正（既存処理を維持）
	auto& look = LookState();
	auto& fov = FovState();
	auto& shake = ShakeState();
	auto& col = CollisionState();

	m_targetRotation = GetRotationQuaternion();
	m_rotation = Math::Quaternion::Slerp(m_prevRotation, m_targetRotation, look.rotationSmooth * deltaTime);
	m_mRotation = Math::Matrix::CreateFromQuaternion(m_rotation);

	// FOV シェイク補間
	fov.fovShake = { fov.fov, 0.0f };
	fov.fovShakeTarget = Math::Vector2::Lerp(fov.fovShakeTarget, fov.fovShake, deltaTime);

	// カメラシェイク
	Math::Vector3 shakeOffset = Math::Vector3::Zero;
	if (shake.shakeTime > 0.0f)
	{
		shakeOffset.x = KdRandom::GetFloat(-shake.shakePower.x, shake.shakePower.x);
		shakeOffset.y = KdRandom::GetFloat(-shake.shakePower.y, shake.shakePower.y);
		shake.shakeTime -= deltaTime;
		if (shake.shakeTime <= 0.0f)
		{
			shake.shakeTime = 0.0f;
			shakeOffset = Math::Vector3::Zero;
		}
	}

	// 追従ターゲット補間（理想）
	look.targetLookAt = Math::Vector3::Lerp(look.targetLookAt, look.followRate, look.distanceSmooth * deltaTime);

	// プレイヤー基準位置
	Math::Vector3 playerPos = m_spTarget->GetPos() + shakeOffset;
	look.cameraPos = Math::Vector3::Lerp(look.cameraPos, playerPos, look.distanceSmooth * deltaTime);

	// 希望カメラ行列（まだ確定でない）
	m_mWorld = Math::Matrix::CreateTranslation(col.effectiveLookAt);
	m_mWorld = m_mWorld * m_mRotation;
	m_mWorld.Translation(m_mWorld.Translation() + look.cameraPos);

	// 衝突補正
	{
		const Math::Vector3 desiredCamWorldPos = m_mWorld.Translation();
		SetPos(desiredCamWorldPos);

		UpdateCameraRayCast(); // col.isBlocked を更新

		const Math::Vector3 correctedCamWorldPos = GetPos();
		const Math::Vector3 effectiveWorldOffset = correctedCamWorldPos - look.cameraPos;

		Math::Matrix invRot = m_mRotation.Invert();
		const Math::Vector3 measuredLocalOffset = Math::Vector3::Transform(effectiveWorldOffset, invRot);

		if (col.isBlocked)
		{
			col.effectiveLookAt = measuredLocalOffset;
		}
		else
		{
			col.effectiveLookAt = Math::Vector3::Lerp(col.effectiveLookAt, look.targetLookAt, look.distanceSmooth * deltaTime);
		}

		look.isBlocked = col.isBlocked;
		m_mWorld.Translation(correctedCamWorldPos);
	}

	m_spCamera->SetCameraMatrix(m_mWorld);
	m_prevRotation = m_rotation;
}

void PlayerCamera::UpdateWinnerCamera()
{
	Application::Instance().SetFpsScale(0.0f);

	KdShaderManager::Instance().m_postProcessShader.SetEnableStrongBlur(false);

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

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	auto& win = WinnerState();
	auto& look = LookState();

	win.time += deltaTime;

	// 区間[0,1)
	if (win.time >= 0.0f)
	{
		m_degree = { -20.0f, 263.0f, 1.0f };
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
		m_degree = { 1.5f, 227.0f, 0.0f };
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
		m_degree = { 10.0f, 320.0f, 0.0f };
		const Math::Vector3 startFollow = { 0.6f, 1.0f, -1.6f };
		const Math::Vector3 endFollow = { 0.6f, 1.0f, -1.5f };
		float t = std::clamp(win.time - 2.0f, 0.0f, 1.0f);
		look.targetLookAt = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		if (win.time >= 2.9f && win.time <= 2.99f)
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);
		else
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
	}

	// 区間[3,∞)
	if (win.time >= 3.0f)
	{
		m_degree = { 0.0f, 90.0f, 0.0f };
		const Math::Vector3 startFollow = { 0.0f, 0.7f, -4.0f };
		const Math::Vector3 endFollow = { 0.0f, 0.7f,  0.0f };
		float t = std::clamp(win.time - 3.0f, 0.0f, 1.0f);
		look.targetLookAt = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);

		if (look.targetLookAt.z >= 0.0f)
		{
			SceneManager::Instance().m_gameClear = false;
			win.time = 0.0f;
			Application::Instance().SetFpsScale(1.0f);
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
		}
	}
}

void PlayerCamera::NewUpdateIntroCamera()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	auto& look = LookState();
	auto& intro = IntroState();
	const IntroConfig& cfg = m_introConfig;

	// 回転
	m_degree.y += cfg.yawSpeedDegPerSec * deltaTime;

	const Math::Vector3 startFollow = cfg.startFollow;
	const Math::Vector3 endFollow = cfg.endFollow;

	if (!intro.inited)
	{
		m_degree.y = cfg.startYawDeg;
		intro.startYaw = m_degree.y;
		intro.inited = true;
		look.followRate = startFollow;
	}

	// 0除算回避
	float denom = std::max(1e-4f, cfg.endYawDeg - intro.startYaw);
	float total = std::clamp((m_degree.y - intro.startYaw) / denom, 0.0f, 1.0f);

	look.followRate = Math::Vector3::SmoothStep(startFollow, endFollow, total);

	if (m_degree.y >= cfg.endYawDeg)
	{
		m_degree.y = cfg.endYawDeg;
		look.followRate = endFollow;

		intro.introTimer += deltaTime;

		if (intro.introTimer >= cfg.holdSeconds)
		{
			// 終了後の伸ばし（Zのみを目標値に向ける）
			look.followRate = { endFollow.x, endFollow.y, cfg.afterEndFollowZ };

			if (look.followRate.z <= cfg.afterEndFollowZ)
			{
				intro.inited = false;
				intro.introTimer = 0.0f;
				SceneManager::Instance().SetIntroCamera(false);
			}
		}
	}
}

void PlayerCamera::ImGuiInspector()
{
	CameraBase::ImGuiInspector();

	auto& look = LookState();
	auto& fov = FovState();

	ImGui::Text(U8("カメラの位置"));
	ImGui::DragFloat3("CameraPos", &look.cameraPos.x, 0.1f);

	ImGui::Text(U8("プレイヤーとカメラの距離"));
	ImGui::DragFloat3("offsetPos", &look.followRate.x, 0.1f);
	ImGui::DragFloat("Camera Smooth", &look.distanceSmooth, 0.01f);
	ImGui::DragFloat("Rotation Smooth", &look.rotationSmooth, 0.01f);
	ImGui::DragFloat("FOV", &fov.fov, 1.0f, 1.0f, 179.0f);

	if (ImGui::Button(U8("カメラのイントロシーンを再生")))
	{
		// フラグ ON
		SceneManager::Instance().SetIntroCamera(true);
		// ステート強制遷移
		ChangeState(std::make_shared<PlayerCameraState_IntroCamera>());
	}

	if (ImGui::Button(U8("カメラのゲームクリアシーンを再生")))
	{
		SceneManager::Instance().m_gameClear = true;
		// Winner 演出ステートへ
		ChangeState(std::make_shared<PlayerCameraState_WinnerCamera>());
	}

	// イントロ設定 GUI
	if (ImGui::CollapsingHeader(U8("Intro Camera Config"), ImGuiTreeNodeFlags_DefaultOpen))
	{
		auto& cfg = IntroConfigRef();
		ImGui::DragFloat(U8("Start Yaw (deg)"), &cfg.startYawDeg, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat(U8("End Yaw (deg)"), &cfg.endYawDeg, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat(U8("Yaw Speed (deg/s)"), &cfg.yawSpeedDegPerSec, 1.0f, 0.0f, 720.0f);
		ImGui::DragFloat3(U8("Start Follow"), &cfg.startFollow.x, 0.01f);
		ImGui::DragFloat3(U8("End Follow"), &cfg.endFollow.x, 0.01f);
		ImGui::DragFloat(U8("Hold Seconds"), &cfg.holdSeconds, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat(U8("AfterEnd Z"), &cfg.afterEndFollowZ, 0.01f, -10.0f, 0.0f);

		ImGui::Separator();
		ImGui::Text(U8("現在Yaw: %.2f"), m_degree.y);
	}

	m_spCamera->SetProjectionMatrix(fov.fov);
}

void PlayerCamera::JsonSave(nlohmann::json& _json) const
{
	CameraBase::JsonSave(_json);
	const auto& look = LookState();
	const auto& fov = FovState();

	_json["targetLookAt"] = JSON_MANAGER.VectorToJson(look.targetLookAt);
	_json["cameraPos"] = JSON_MANAGER.VectorToJson(look.cameraPos);
	_json["smooth"] = look.distanceSmooth;
	_json["rotationSmooth"] = look.rotationSmooth;
	_json["fov"] = fov.fov;

	// イントロ設定保存
	const auto& cfg = m_introConfig;
	auto& intro = _json["intro"];
	intro["startYawDeg"] = cfg.startYawDeg;
	intro["endYawDeg"] = cfg.endYawDeg;
	intro["yawSpeedDegPerSec"] = cfg.yawSpeedDegPerSec;
	intro["startFollow"] = JSON_MANAGER.VectorToJson(cfg.startFollow);
	intro["endFollow"] = JSON_MANAGER.VectorToJson(cfg.endFollow);
	intro["holdSeconds"] = cfg.holdSeconds;
	intro["afterEndFollowZ"] = cfg.afterEndFollowZ;
}

void PlayerCamera::JsonInput(const nlohmann::json& _json)
{
	CameraBase::JsonInput(_json);

	auto& look = LookState();
	auto& fov = FovState();

	if (_json.contains("targetLookAt"))   look.targetLookAt = JSON_MANAGER.JsonToVector(_json["targetLookAt"]);
	if (_json.contains("cameraPos"))      look.cameraPos = JSON_MANAGER.JsonToVector(_json["cameraPos"]);
	if (_json.contains("smooth"))         look.distanceSmooth = _json["smooth"].get<float>();
	if (_json.contains("rotationSmooth")) look.rotationSmooth = _json["rotationSmooth"].get<float>();
	if (_json.contains("fov"))            fov.fov = _json["fov"].get<float>();

	// イントロ設定読込
	if (_json.contains("intro"))
	{
		const auto& intro = _json["intro"];
		auto& cfg = m_introConfig;

		if (intro.contains("startYawDeg"))       cfg.startYawDeg = intro["startYawDeg"].get<float>();
		if (intro.contains("endYawDeg"))         cfg.endYawDeg = intro["endYawDeg"].get<float>();
		if (intro.contains("yawSpeedDegPerSec")) cfg.yawSpeedDegPerSec = intro["yawSpeedDegPerSec"].get<float>();
		if (intro.contains("startFollow"))       cfg.startFollow = JSON_MANAGER.JsonToVector(intro["startFollow"]);
		if (intro.contains("endFollow"))         cfg.endFollow = JSON_MANAGER.JsonToVector(intro["endFollow"]);
		if (intro.contains("holdSeconds"))       cfg.holdSeconds = intro["holdSeconds"].get<float>();
		if (intro.contains("afterEndFollowZ"))   cfg.afterEndFollowZ = intro["afterEndFollowZ"].get<float>();
	}
}

void PlayerCamera::ChangeState(std::shared_ptr<PlayerCameraState> _state)
{
	_state->SetPlayerCamera(this);
	m_stateManager.ChangeState(_state);
}

void PlayerCamera::StateInit()
{
	ChangeState(std::make_shared<PlayerCameraState_IntroCamera>());
}

void PlayerCamera::UpdateCameraRayCast()
{
	KdCollider::RayInfo rayInfo;
	rayInfo.m_pos = GetPos();
	rayInfo.m_dir = Math::Vector3::Down;
	rayInfo.m_range = 1000.f;

	if (m_spTarget)
	{
		Math::Vector3 _targetPos = m_spTarget->GetPos() + Math::Vector3{ 0, 1.0f, 0 };
		_targetPos.y += 0.1f;
		rayInfo.m_dir = _targetPos - rayInfo.m_pos;
		rayInfo.m_range = rayInfo.m_dir.Length();
		rayInfo.m_dir.Normalize();
	}

	rayInfo.m_type = KdCollider::TypeCameraHit;

	bool anyHit = false;
	float bestDistSq = 1e30f;
	Math::Vector3 bestHitPos = {};

	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		if (auto spGameObj = wpGameObj.lock())
		{
			std::list<KdCollider::CollisionResult> retRayList;
			spGameObj->Intersects(rayInfo, &retRayList);

			for (const auto& ret : retRayList)
			{
				const Math::Vector3 v = ret.m_hitPos - rayInfo.m_pos;
				const float distSq = v.LengthSquared();

				if (distSq < bestDistSq)
				{
					bestDistSq = distSq;
					bestHitPos = ret.m_hitPos;
					anyHit = true;
				}
			}
		}
	}

	auto& col = CollisionState();
	auto& look = LookState();

	if (anyHit)
	{
		col.isBlocked = true;
		look.isBlocked = true;

		Math::Vector3 _hitPos = bestHitPos;
		SetPos(_hitPos);
	}
	else
	{
		col.isBlocked = false;
		look.isBlocked = false;
	}
}