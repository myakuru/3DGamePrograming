#include "PlayerCamera.h"
#include"../../../main.h"
#include"../../../Scene/SceneManager.h"
#include"../../Character/Player/Player.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../Utility/Time.h"
#include"PlayerCameraState\PlayerCameraState.h"


const uint32_t PlayerCamera::TypeID = KdGameObject::GenerateTypeID();

void PlayerCamera::Init()
{
	CameraBase::Init();

	// エフェクシアのカメラをセット
	KdEffekseerManager::GetInstance().SetCamera(m_spCamera);

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	// 起動時にカーソルを動かさない。現在位置を保持
	GetCursorPos(&m_FixMousePos);

	// 初期のカメラのターゲット座標を設定
	m_followRate = { 0.0f, 1.0f, -0.5f };

	m_fovShake = { m_fov,0.0f };

	m_fovShakeTarget = m_fovShake;

	m_spCamera->SetProjectionMatrix(m_fovShakeTarget.x);

	StateInit();

	SceneManager::Instance().GetObjectWeakPtr(m_Player);

	if (auto player = m_Player.lock(); player)
	{
		m_cameraPos = player->GetPos();
	}

	m_degree = { 0.0f,140.0f,0.0f };

	m_startYaw = 0.0f;
}

void PlayerCamera::PostUpdate()
{
	if (SceneManager::Instance().m_sceneCamera) return;

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	m_spTarget = m_Player.lock();
	if (!m_spTarget) return;

	// 回転更新
	UpdateRotateByMouse();

	m_stateManager.Update();

	m_targetRotation = GetRotationQuaternion();
	m_rotation = Math::Quaternion::Slerp(m_prevRotation, m_targetRotation, m_rotationSmooth * deltaTime);
	m_mRotation = Math::Matrix::CreateFromQuaternion(m_rotation);

	m_fovShake = { m_fov,0.0f };

	m_fovShakeTarget = Math::Vector2::Lerp(m_fovShakeTarget, m_fovShake, deltaTime);

	// シェイク
	Math::Vector3 shakeOffset = Math::Vector3::Zero;
	if (m_shakeTime > 0.0f)
	{
		shakeOffset.x = KdRandom::GetFloat(-m_shakePower.x, m_shakePower.x);
		shakeOffset.y = KdRandom::GetFloat(-m_shakePower.y, m_shakePower.y);
		m_shakeTime -= deltaTime;
		if (m_shakeTime <= 0.0f) { m_shakeTime = 0.0f; shakeOffset = Math::Vector3::Zero; }
	}

	// 追従ターゲット補間
	m_targetLookAt = Math::Vector3::Lerp(m_targetLookAt, m_followRate, m_dhistanceSmooth * deltaTime);

	// プレイヤー基準位置
	Math::Vector3 playerPos = m_spTarget->GetPos() + shakeOffset;

	m_cameraPos = Math::Vector3::Lerp(m_cameraPos, playerPos, m_dhistanceSmooth * deltaTime);

	// 一旦希望のカメラ行列を組み立て（まだ確定ではない）
	m_mWorld = Math::Matrix::CreateTranslation(m_targetLookAt);      // 注視点用オフセット
	m_mWorld = m_mWorld * m_mRotation;
	m_mWorld.Translation(m_mWorld.Translation() + m_cameraPos);

	// プレイヤーの「視点アンカー」(頭～胸あたり) を設定
	Math::Vector3 anchor = playerPos + Math::Vector3(0, 1.0f, 0); // 必要なら 1.2f などチューニング

	// 障害物補正
	//UpdateCameraRayCast(anchor);
	if (SceneManager::Instance().IsIntroCamera())
	{
		NewUpdateIntroCamera();
	}

	if (SceneManager::Instance().m_gameClear && !SceneManager::Instance().IsIntroCamera())
	{
		//UpdateWinnerCamera();
	}

	// 最終的にカメラ行列適用
	m_spCamera->SetCameraMatrix(m_mWorld);

	m_prevRotation = m_rotation;
}

void PlayerCamera::UpdateWinnerCamera()
{
	Application::Instance().SetFpsScale(0.0f); // 一時停止（意図が「停止」ならこのまま。※「リセット」なら 1.0f）

	KdShaderManager::Instance().m_postProcessShader.SetEnableStrongBlur(false);

	float time = Time::Instance().GetElapsedTime();
	int sec = static_cast<int>(time);

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

	// Unscaled はフレーム末に更新されるので、ここでの値は「前フレーム分」
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	m_time += deltaTime;

	// 区間 [0,1)
	if (m_time >= 0.0f)
	{
		m_degree = { -20.0f,263.0f,1.0f };
		const Math::Vector3 startFollow = { 0.0f, 0.7f, -1.0f };
		const Math::Vector3 endFollow = { 0.0f, 0.7f, -0.9f };
		float t = std::clamp(m_time, 0.0f, 1.0f);
		m_followRate = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		if (m_time >= 0.9f && m_time <= 0.99f)
		{
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);
		}
		else
		{
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
		}
	}

	// 区間 [1,2)
	if (m_time >= 1.0f)
	{
		m_degree = { 1.5f,227.0f,0.0f };
		const Math::Vector3 startFollow = { 3.4f, 1.0f, -4.4f };
		const Math::Vector3 endFollow = { 3.4f, 1.0f, -4.3f };
		float t = std::clamp((m_time - 1.0f), 0.0f, 1.0f);
		m_targetLookAt = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		if (m_time >= 1.9f && m_time <= 1.99f)
		{
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);
		}
		else
		{
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
		}
	}

	// 区間 [2,3)
	if (m_time >= 2.0f)
	{
		m_degree = { 10.0f,320.0f,0.0f };
		const Math::Vector3 startFollow = { 0.6f, 1.0f, -1.6f };
		const Math::Vector3 endFollow = { 0.6f, 1.0f, -1.5f };
		float t = std::clamp((m_time - 2.0f), 0.0f, 1.0f);
		m_targetLookAt = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		if (m_time >= 2.9f && m_time <= 2.99f)
		{
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);
		}
		else
		{
			KdShaderManager::Instance().m_postProcessShader.SetEnableGray(false);
		}
	}

	// 区間 [3,∞)
	if (m_time >= 3.0f)
	{
		m_degree = { 0.0f,90.0f,0.0f };
		const Math::Vector3 startFollow = { 0.0f, 0.7f, -4.0f };
		const Math::Vector3 endFollow = { 0.0f, 0.7f,  0.0f };
		float t = std::clamp((m_time - 3.0f), 0.0f, 1.0f);
		m_targetLookAt = Math::Vector3::SmoothStep(startFollow, endFollow, t);

		KdShaderManager::Instance().m_postProcessShader.SetEnableGray(true);

		if (m_targetLookAt.z >= 0.0f)
		{
			SceneManager::Instance().m_gameClear = false;
			m_time = 0.0f;

			Application::Instance().SetFpsScale(1.0f); // 解除
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
		}
	}
}

void PlayerCamera::NewUpdateIntroCamera()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	// 回転
	m_degree.y += 60.0f * deltaTime;

	// フォロー距離の補間：-0.5f -> -1.7f を 320度 到達までに完了
	const Math::Vector3 startFollow = { 0.0f, 1.0f, -0.5f };
	const Math::Vector3 endFollow = { 0.0f, 1.0f, -1.5f };

	// 開始角を記録（初回のみ）
	if (!m_inited)
	{
		m_degree.y = 145.0f; // 安全策でリセット
		m_startYaw = m_degree.y;
		m_inited = true;
		m_followRate = startFollow; // 初期値を明示
	}

	
	// 補間区間の“全長” •	開始角 m_startYaw から目標角 320.0f までの差分を total = 320.0f - m_startYaw
	float denom = std::max(1e-4f, 320.0f - m_startYaw);
	// 現在の進捗度合い t = (m_degree.y - m_startYaw) / total を 0.0f ～ 1.0f にクランプ
	float total = std::clamp((m_degree.y - m_startYaw) / denom, 0.0f, 1.0f);

	m_followRate = Math::Vector3::SmoothStep(startFollow, endFollow, total);

	// 到達以降の処理
	if (m_degree.y >= 320.0f)
	{
		m_degree.y = 320.0f;
		m_followRate = endFollow; // 到達時に確定

		m_time += deltaTime;

		if (m_time >= 1.0f)
		{
			// 余韻で更に引く
			m_followRate = { 0.0f, 1.0f, -2.5f };

			if (m_followRate.z <= -2.5f)
			{
				// リセット（次回イントロ開始のため）
				m_inited = false;
				m_time = 0.0f;
				SceneManager::Instance().SetIntroCamera(false);
			}
		}
	}
}

void PlayerCamera::ImGuiInspector()
{
	CameraBase::ImGuiInspector();

	ImGui::Text(U8("カメラの位置"));
	ImGui::DragFloat3("CameraPos", &m_cameraPos.x, 0.1f);

	ImGui::Text(U8("プレイヤーとカメラの距離"));
	ImGui::DragFloat3("offsetPos", &m_followRate.x, 0.1f);
	ImGui::DragFloat("Camera Smooth", &m_dhistanceSmooth, 0.01f);
	ImGui::DragFloat("Rotation Smooth", &m_rotationSmooth, 0.01f);
	ImGui::DragFloat("FOV", &m_fov, 1.0f, 1.0f, 179.0f);

	// イントロカメラリセットボタン
	if (ImGui::Button(U8("カメラのイントロシーンを再生")))
	{
		SceneManager::Instance().SetIntroCamera(true);
	}

	// ゲームクリアカメラリセットボタン
	if (ImGui::Button(U8("カメラのゲームクリアシーンを再生")))
	{
		SceneManager::Instance().m_gameClear = true;
	}


	m_spCamera->SetProjectionMatrix(m_fov);
}

void PlayerCamera::JsonSave(nlohmann::json& _json) const
{
	CameraBase::JsonSave(_json);
	_json["targetLookAt"] = JSON_MANAGER.VectorToJson(m_targetLookAt);
	_json["m_cameraPos"] = JSON_MANAGER.VectorToJson(m_cameraPos);
	_json["smooth"] = m_dhistanceSmooth;
	_json["rotationSmooth"] = m_rotationSmooth;
	_json["fov"] = m_fov;
}

void PlayerCamera::JsonInput(const nlohmann::json& _json)
{
	CameraBase::JsonInput(_json);
	if(_json.contains("targetLookAt")) m_targetLookAt = JSON_MANAGER.JsonToVector(_json["targetLookAt"]);
	if (_json.contains("m_cameraPos")) m_cameraPos = JSON_MANAGER.JsonToVector(_json["m_cameraPos"]);
	if (_json.contains("smooth")) m_dhistanceSmooth = _json["smooth"].get<float>();
	if (_json.contains("rotationSmooth")) m_rotationSmooth = _json["rotationSmooth"].get<float>();
	if (_json.contains("fov")) m_fov = _json["fov"].get<float>();
}

DirectX::BoundingFrustum PlayerCamera::CreateFrustum() const
{
	DirectX::BoundingFrustum frustum;
	DirectX::BoundingFrustum::CreateFromMatrix(frustum, m_spCamera->GetProjMatrix());
	// カメラのワールド行列で変換
	frustum.Transform(frustum, m_spCamera->GetCameraMatrix());

	return frustum;
}

void PlayerCamera::DebugDraw(DirectX::BoundingFrustum _frustum)
{

	// 8つのコーナー座標を取得
	DirectX::XMFLOAT3 corners[8];
	_frustum.GetCorners(corners);

	// 視錐台の12本のエッジを線で描画
	auto addLine = [this, &corners](int i0, int i1) {
		m_pDebugWire->AddDebugLine(
			Math::Vector3(corners[i0].x, corners[i0].y, corners[i0].z),
			Math::Vector3(corners[i1].x, corners[i1].y, corners[i1].z),
			Math::Color(1, 1, 0, 1) // 黄色など任意の色
		);
		};

	// エッジのインデックス（Near面:0-3, Far面:4-7）
	const int edgeIndices[12][2] = {
		{0,1},{1,2},{2,3},{3,0}, // Near
		{4,5},{5,6},{6,7},{7,4}, // Far
		{0,4},{1,5},{2,6},{3,7}  // Side
	};
	for (const auto& edge : edgeIndices) {
		addLine(edge[0], edge[1]);
	}
}

void PlayerCamera::UpdateCameraRayCast(const Math::Vector3& _anchor)
{
	// 希望するカメラの最終ワールド座標(回転など反映済み)
	Math::Vector3 desiredCamPos = m_mWorld.Translation();

	Math::Vector3 rayDir = desiredCamPos - _anchor;
	float baseDist = rayDir.Length();
	if (baseDist < 0.0001f) return;
	rayDir.Normalize();

	// 初期化
	if (m_currentCamDistance < 0.0f)
	{
		m_currentCamDistance = baseDist;
		m_prevHitDist = baseDist;
		m_hitDistSmoothing = 0.35f;
	}

	KdCollider::RayInfo rayInfo;
	rayInfo.m_pos = _anchor;
	rayInfo.m_dir = rayDir;
	rayInfo.m_range = baseDist;
	rayInfo.m_type = KdCollider::TypeBump;

	float nearestRawHitDist = baseDist;
	bool  hit = false;

	// 最接近ヒット探索
	for (auto& wpObj : m_wpHitObjectList)
	{
		auto obj = wpObj.lock();
		if (!obj) continue;
		if (obj.get() == this) continue;
		if (m_spTarget && obj.get() == m_spTarget.get()) continue;

		std::list<KdCollider::CollisionResult> hitList;
		obj->Intersects(rayInfo, &hitList);
		for (auto& h : hitList)
		{
			float d = (h.m_hitPos - rayInfo.m_pos).Length();
			if (d < nearestRawHitDist)
			{
				nearestRawHitDist = d;
				hit = true;
			}
		}
	}

	// ヒット距離ノイズ抑制（小さな揺れを吸収）
	if (hit)
	{

		// 平滑化(指数移動平均)
		float w = m_hitDistSmoothing; // 0.0～1.0
		nearestRawHitDist = m_prevHitDist * (1.0f - w) + nearestRawHitDist * w;
		m_prevHitDist = nearestRawHitDist;
	}

	// 目標距離決定
	float targetDist = baseDist;
	if (hit)
	{
		targetDist = std::min(baseDist, nearestRawHitDist - m_obstacleMargin);
	}

	// 近づく / 離れる で違う速度
	float dt = Application::Instance().GetDeltaTime();
	bool shrinking = (targetDist < m_currentCamDistance - 0.0001f);

	float speed = shrinking ? m_camHitSmoothIn : m_camHitSmoothOut;
	// 指数平滑(フレームレート非依存化)
	float alpha = 1.0f - std::exp(-speed * dt);
	m_currentCamDistance = m_currentCamDistance + (targetDist - m_currentCamDistance) * alpha;

	// 最終カメラ位置更新
	Math::Vector3 newCamPos = _anchor + rayDir * m_currentCamDistance;
	Math::Vector3 oldPos = m_mWorld.Translation();
	m_mWorld.Translation(newCamPos);

	// 既存 m_cameraPos へ差分反映（他処理との整合性維持）
	m_cameraPos += (newCamPos - oldPos);
}

void PlayerCamera::ChangeState(std::shared_ptr<PlayerCameraState> _state)
{
	_state->SetPlayerCamera(this);
	m_stateManager.ChangeState(_state);
}

void PlayerCamera::StateInit()
{
	//m_stateManager.ChangeState(std::make_shared<PlayerCameraState>());
}

//void PlayerCamera::UpdateCameraRayCast()
//{
//	// ↓めり込み防止の為の座標補正計算↓
//	// ①当たり判定(レイ判定)用の情報作成
//	KdCollider::RayInfo rayInfo;
//	// レイの発射位置を設定
//	rayInfo.m_pos = GetPos();
//
//	// レイの発射方向を設定
//	rayInfo.m_dir = Math::Vector3::Down;
//	// レイの長さを設定
//	rayInfo.m_range = 1000.f;
//	if (m_spTarget)
//	{
//		Math::Vector3 _targetPos = m_spTarget->GetPos() + Math::Vector3{ 0, 1.0f, 0 };
//		_targetPos.y += 0.1f;
//		rayInfo.m_dir = _targetPos - GetPos();
//		rayInfo.m_range = rayInfo.m_dir.Length();
//		rayInfo.m_dir.Normalize();
//	}
//
//	// 当たり判定をしたいタイプを設定
//	rayInfo.m_type = KdCollider::TypeBump;
//
//	// ②HIT判定対象オブジェクトに総当たり
//	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
//	{
//		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
//		if (spGameObj)
//		{
//			std::list<KdCollider::CollisionResult> retRayList;
//			spGameObj->Intersects(rayInfo, &retRayList);
//
//			// ③ 結果を使って座標を補完する
//			// レイに当たったリストから一番近いオブジェクトを検出
//			float maxOverLap = 0;
//			Math::Vector3 hitPos = {};
//			bool hit = false;
//			for (auto& ret : retRayList)
//			{
//				// レイを遮断しオーバーした長さが
//				// 一番長いものを探す
//				if (maxOverLap < ret.m_overlapDistance)
//				{
//					maxOverLap = ret.m_overlapDistance;
//					hitPos = ret.m_hitPos;
//					hit = true;
//				}
//			}
//			if (hit)
//			{
//				// 何かしらの障害物に当たっている
//				Math::Vector3 _hitPos = hitPos;
//				_hitPos += rayInfo.m_dir * 0.4f;
//				SetPos(_hitPos);
//			}
//		}
//	}
//}
