#include "TPSCamera.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"

const uint32_t TPSCamera::TypeID = KdGameObject::GenerateTypeID();

void TPSCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 2.f, -2.0f);

	// 起動時にカーソルを動かさない。現在位置を保持
	GetCursorPos(&m_FixMousePos);
}

void TPSCamera::PostUpdate()
{
	if (!SceneManager::Instance().m_sceneCamera)
	{
		// カメラの回転
		UpdateRotateByMouse();
		m_mRotation = GetRotationMatrix();
		UpdateMoveKey();

		UpdateResultCamera();
	}
}

void TPSCamera::UpdateResultCamera()
{
	if (SceneManager::Instance().GetResultFlag())
	{
		float deltaTime = Application::Instance().GetDeltaTime();

		// 初期化用 static 変数
		static bool initialized = false;
		if (!initialized)
		{
			m_position.x = 141.0f;
			initialized = true;
		}

		// x座標を減算して移動
		m_position.x -= moveSpeed * deltaTime;

		// x座標が -8.6 より小さくなったら 141 に戻す
		if (m_position.x <= -8.6f)
		{
			m_position.x = 141.0f;
		}

		m_mWorld = m_mLocalPos * m_mRotation;
		m_mWorld.Translation(m_position);
		m_spCamera->SetCameraMatrix(m_mWorld);
	}
	else
	{
		// ResultFlagがfalseになったら初期化フラグをリセット
		static bool initialized = false;
		initialized = false;
	}

	return;
}
