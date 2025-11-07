#include "SkillBar.h"
#include"../../../Scene/SceneManager.h"
#include"../../../Data/CharacterData/CharacterData.h"
#include"../../../main.h"

#include"../../Character/Player/Player.h"

const uint32_t SkillBar::TypeID = KdGameObject::GenerateTypeID();

void SkillBar::Init()
{
	SelectDraw2DTexture::Init();

	// プレイヤーデータ取得
	SceneManager::Instance().GetObjectWeakPtr(m_player);
}

void SkillBar::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;

	// 現在のビューポートサイズ取得
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);

	// スケーリング
	const float sx = vp.width / kRefW;
	const float sy = vp.height / kRefH;

	Math::Matrix uiScale = Math::Matrix::CreateScale(sx, sy, 1.0f);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(m_mWorld * uiScale);

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_texture,
		static_cast<int>(m_position.x), // 1920x1080基準の座標そのまま
		static_cast<int>(m_position.y),
		m_rect.width,
		m_rect.height,
		&m_rect,
		&m_color,
		{ 0.0f, 0.5f } // 左端基準で縮む
	);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
}

void SkillBar::Update()
{

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	m_timer += deltaTime;



	// 目標のHP割合を算出（0～1にクランプ）
	float hpRate = 0.0f;
	
	if (auto playerStatus = m_player.lock(); playerStatus)
	{
		if (playerStatus->GetStatus().GetPlayerStatus().skillPointMax > 0)
		{
			hpRate = static_cast<float>(playerStatus->GetStatus().GetPlayerStatus().skillPoint) / static_cast<float>(playerStatus->GetStatus().GetPlayerStatus().skillPointMax);
		}
		else
		{
			hpRate = 1.0f;
		}

		// 表示幅を更新
		m_rect.width = static_cast<long>(500.0f * hpRate);

		if (playerStatus->GetStatus().GetPlayerStatus().skillPoint >= 30)
		{
			if (m_timer >= 0.1f)
			{
				// 赤 or 紫をランダムに選択
				const bool useRed = KdRandom::GetFloat(0.0f, 1.0f) < 0.5f;

				float r, g, b;
				if (useRed)
				{
					// 赤系: R高 / G低 / B低
					r = KdRandom::GetFloat(0.9f, 1.0f);
					g = KdRandom::GetFloat(0.0f, 0.2f);
					b = KdRandom::GetFloat(0.0f, 0.2f);
				}
				else
				{
					// 紫系: R高 / G低 / B高
					r = KdRandom::GetFloat(0.8f, 1.0f);
					g = KdRandom::GetFloat(0.0f, 0.2f);
					b = KdRandom::GetFloat(0.8f, 1.0f);
				}

				m_color = { r, g, b, 1.0f };
				m_timer = 0.0f;
			}
		}
		else
		{
			// 通常色に戻す
			m_color = { 0.5f, 0.5f, 0.5f, 1.0f };
		}
	}

	


	// 行列更新
	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll
	(
		m_degree.y,
		m_degree.x,
		m_degree.z
	);
	m_mWorld.Translation(m_position);
}
