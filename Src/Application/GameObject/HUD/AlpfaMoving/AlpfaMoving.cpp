#include "AlpfaMoving.h"
#include"../../../main.h"
#include"../../../Scene/SceneManager.h"
const uint32_t AlpfaMoving::TypeID = KdGameObject::GenerateTypeID();

void AlpfaMoving::Init()
{
	SelectDraw2DTexture::Init();
}

void AlpfaMoving::Update()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// sin関数でスケールを変化させる

	// (0.5f~1.0f)の間で変化させる
	// 最小値: 0.75 - 0.25 = 0.5f
	// 最大値: 0.75 + 0.25 = 1.0f

	m_color = { 1.0f, 1.0f, 1.0f, m_alpha }; // 白色
	m_min = 0.2f;
	m_max = 1.0f;
	m_alpha = m_min + (m_max - m_min) * 0.5f * (sinf(m_time * 2.0f) + 1.0f);

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld.Translation(m_position);
}

void AlpfaMoving::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;
	SelectDraw2DTexture::DrawSprite();
}
