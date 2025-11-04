#include "BlackBanner.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"

const uint32_t BlackBanner::TypeID = KdGameObject::GenerateTypeID();

void BlackBanner::Init()
{
	SelectDraw2DTexture::Init();
	m_scale.y = 1.3f;

}

void BlackBanner::Update()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	if (SceneManager::Instance().GetResultFlag() || SceneManager::Instance().IsIntroCamera())
	{

		if (m_scale.y > 1.0f)
		{
			m_scale.y -= 1.f * deltaTime;	// 徐々に縮小。
		}
		else
		{
			m_scale.y = 1.0f;	// 最小値を設定。
		}
	}
	else
	{
		if (m_scale.y < 1.3f)
		{
			m_scale.y += 1.f * deltaTime;	// 徐々に拡大。
		}
		else
		{
			m_scale.y = 1.3f;	// 最大値を設定。
		}
	}

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld.Translation(m_position);
}

void BlackBanner::DrawSprite()
{
	if (m_scale.y <= 1.3f)
	{
		SelectDraw2DTexture::DrawSprite();
	}
}
