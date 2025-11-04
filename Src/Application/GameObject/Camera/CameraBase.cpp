#include "CameraBase.h"
#include"../../main.h"

void CameraBase::Init()
{
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}
}

void CameraBase::PreDraw()
{
	if (!m_spCamera) { return; }
	m_spCamera->SetToShader();
}

void CameraBase::ImGuiInspector()
{
	KdGameObject::ImGuiInspector();

	if (!m_spCamera) { return; }
	ImGui::DragFloat("moveSpeed", &moveSpeed, 1.0f);

}

void CameraBase::JsonSave(nlohmann::json& _json) const
{
	KdGameObject::JsonSave(_json);
	_json["moveSpeed"] = moveSpeed;
}

void CameraBase::JsonInput(const nlohmann::json& _json)
{
	KdGameObject::JsonInput(_json);
	if (_json.contains("moveSpeed")) moveSpeed = _json["moveSpeed"].get<float>();
}

void CameraBase::UpdateMoveKey()
{
	if (!m_enabled) return;

	float deltaTime = Application::Instance().GetDeltaTime();

	// 回転行列から前方向と右方向を取得
	Math::Vector3 backward = m_mRotation.Backward();
	Math::Vector3 right = m_mRotation.Right();
	float moveUpSpeed = 10.0f; // 上下移動速度

	// 斜め移動はやくならないように正規化
	backward.Normalize();
	right.Normalize();

	// 距離の移動量を計算
	Math::Vector3 move = {};

	if (KeyboardManager::GetInstance().IsKeyPressed('W')) move += backward * moveSpeed * deltaTime;
	if (KeyboardManager::GetInstance().IsKeyPressed('A')) move -= right * moveSpeed * deltaTime;
	if (KeyboardManager::GetInstance().IsKeyPressed('S')) move -= backward * moveSpeed * deltaTime;
	if (KeyboardManager::GetInstance().IsKeyPressed('D')) move += right * moveSpeed * deltaTime;
	if (KeyboardManager::GetInstance().IsKeyPressed(VK_SPACE)) move.y += moveUpSpeed * deltaTime; // 上に移動
	if (KeyboardManager::GetInstance().IsKeyPressed(VK_LSHIFT)) move.y -= moveUpSpeed * deltaTime; // 下に移動

	// 距離 = 速度 * 時間
	m_position += move;
}

DirectX::BoundingFrustum CameraBase::CreateFrustum() const
{
	DirectX::BoundingFrustum frustum;
	DirectX::BoundingFrustum::CreateFromMatrix(frustum, m_spCamera->GetProjMatrix());

	frustum.Origin = m_mWorld.Translation();

	// 視錐台の回転をクォータニオンで設定
	frustum.Orientation = Math::Quaternion::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z));

	return frustum;
}

void CameraBase::UpdateRotateByMouse()
{
	static bool prevTab = false; // 前フレームのTABキー状態

	// TABキーの現在の状態
	bool nowTab = (GetAsyncKeyState(VK_TAB) & 0x8000);

	// TABキーが押された瞬間だけトグル
	if (nowTab && !prevTab)
	{
		m_enabled = !m_enabled;

		if (m_enabled)
		{
			// 有効化時に固定点をウィンドウ中央に設定し、一度だけカーソルを移動
			HWND hwnd = Application::Instance().GetWindowHandle();
			RECT rc{};
			GetClientRect(hwnd, &rc);
			POINT center{ (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2 };
			ClientToScreen(hwnd, &center);
			m_FixMousePos = center;
			SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

			SwitchShowCursor(false);
		}
		else
		{
			SwitchShowCursor(true);
		}
	}

	// 有効/無効の維持
	if (m_enabled) SwitchShowCursor(false);
	if (!m_enabled) SwitchShowCursor(true);

	prevTab = nowTab;

	if (m_enabled)
	{
		// マウスでカメラを回転させる処理
		POINT _nowPos;
		GetCursorPos(&_nowPos);

		POINT _mouseMove{};
		_mouseMove.x = _nowPos.x - m_FixMousePos.x;
		_mouseMove.y = _nowPos.y - m_FixMousePos.y;

		// 毎フレーム、固定点に戻す
		SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

		// 実際にカメラを回転させる処理(0.15は補正値)
		m_degree.x += _mouseMove.y * 0.15f;
		m_degree.y += _mouseMove.x * 0.15f;
	}

	// 回転制御
	m_degree.x = std::clamp(m_degree.x, -45.f, 45.f);
	if (m_degree.y > 360.f) m_degree.y -= 360.f;
	if (m_degree.y < 0.f) m_degree.y += 360.f;
}
