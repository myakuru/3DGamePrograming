#include "ChallengeResults.h"
#include"../../Utility/Time.h"
#include"../../../main.h"

const uint32_t ChallengeResults::TypeID = KdGameObject::GenerateTypeID();

void ChallengeResults::Init()
{
	SelectDraw2DTexture::Init();
	m_isComplete = false;

	m_scale = { 1.0f,0.0f,0.0f };
}

void ChallengeResults::DrawSprite()
{
	if (!m_isComplete) return;
	SelectDraw2DTexture::DrawSprite();
}

void ChallengeResults::Update()
{
	float time = Time::Instance().GetElapsedTime();
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (time >= 7.0f)
	{
		m_isComplete = true;

		if (m_scale.y < 1.0f)
		{
			m_scale.y += 50.0f * deltaTime;
		}
		else
		{
			m_scale.y = 1.0f;
		}
	}

	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	);
	m_mWorld.Translation(m_position);
}
