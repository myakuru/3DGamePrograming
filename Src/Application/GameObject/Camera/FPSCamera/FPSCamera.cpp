#include "FPSCamera.h"
#include"../../../Scene/SceneManager.h"

const uint32_t FPSCamera::TypeID = KdGameObject::GenerateTypeID();

void FPSCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 基準点(ターゲット)の目線
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 1.5f, 0.0f);

	// 起動時にカーソルを動かさない。現在位置を保持
	GetCursorPos(&m_FixMousePos);
}

void FPSCamera::CameraUpdate()
{
	if (!SceneManager::Instance().m_sceneCamera) return;
	{
		// カメラの回転
		UpdateRotateByMouse();
		m_mRotation = GetRotationMatrix();
		UpdateMoveKey();
		m_mWorld = m_mLocalPos * m_mRotation;
		m_mWorld.Translation(m_position);
		m_spCamera->SetCameraMatrix(m_mWorld);
	}
}