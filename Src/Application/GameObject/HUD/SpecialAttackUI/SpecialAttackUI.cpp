#include "SpecialAttackUI.h"
#include"Application/Scene/SceneManager.h"
#include"Application/GameObject/Character/Player/Player.h"
#include"Application/Data/CharacterData/CharacterData.h"
#include"Application\main.h"

const uint32_t SpecialAttackUI::TypeID = KdGameObject::GenerateTypeID();

void SpecialAttackUI::Init()
{
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameUI/SpecialMoveCharge.png");
	m_specialAttackIconTexture = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameUI/onSpecialMove.png");

	SceneManager::Instance().GetObjectWeakPtr(m_player);

	m_showSpecialAttackUI = false;
}

void SpecialAttackUI::Update()
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	m_timer += deltaTime;

	if (auto player = m_player.lock(); player)
	{
		// 目標のHP割合を算出（0～1にクランプ）
		if (player->GetStatus().GetPlayerStatus().specialPointMax >= 0)
		{
			m_specialCountRate = 
				std::clamp(static_cast<float>(player->GetStatus().GetPlayerStatus().specialPoint) 
					/ static_cast<float>(player->GetStatus().GetPlayerStatus().specialPointMax), 0.0f, 1.0f);
			m_showSpecialAttackUI = false;
		}

		// 表示幅（可変）
		const long visibleW = std::clamp<long>(static_cast<long>(std::round(100 * m_specialCountRate)), 0, 100);

		// 描画先のサイズ
		m_rect.height = visibleW;

		m_srcRect.x = 0;
		m_srcRect.y = 100 - visibleW;
		m_srcRect.height = visibleW;

		if (m_srcRect.height == 100)
		{
			m_showSpecialAttackUI = true;

			if (m_timer >= 0.1f)
			{
				const float r = KdRandom::GetFloat(0.95f, 1.0f);
				const float g = KdRandom::GetFloat(0.65f, 1.0f);
				const float b = KdRandom::GetFloat(0.00f, 0.15f);

				m_color = { r, g, b, 1.0f };
				m_timer = 0.0f;
			}
		}
		else
		{
			m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
			m_showSpecialAttackUI = false;
		}

	}
}

void SpecialAttackUI::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;

	if (!m_showSpecialAttackUI)
	{

		// 現在のビューポートサイズ取得
		Math::Viewport vp;
		KdDirect3D::Instance().CopyViewportInfo(vp);

		// 伸張（Stretch）：XとYを個別にスケーリング（画面サイズにピッタリ）
		const float sx = vp.width / kRefW;
		const float sy = vp.height / kRefH;

		Math::Matrix uiScale = Math::Matrix::CreateScale(sx, sy, 1.0f);

		// UIスケールをワールド行列に後掛け（平行移動も含め全体をスケール）
		Math::Matrix m = m_mWorld * uiScale;

		KdShaderManager::Instance().m_spriteShader.SetMatrix(m);
		if (m_bDrawTexture)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				m_texture,
				static_cast<int>(m_position.x), // 1920x1080基準の座標そのまま
				static_cast<int>(m_position.y),
				m_rect.width,
				m_rect.height,
				&m_srcRect,
				&m_color,
				{ 0.5f, 0.0f } // 下端基準で縮む
			);
		}
		KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	}
	else
	{
		// 現在のビューポートサイズ取得
		Math::Viewport vp;
		KdDirect3D::Instance().CopyViewportInfo(vp);

		// 伸張（Stretch）：XとYを個別にスケーリング（画面サイズにピッタリ）
		const float sx = vp.width / kRefW;
		const float sy = vp.height / kRefH;

		Math::Matrix uiScale = Math::Matrix::CreateScale(sx, sy, 1.0f);

		// UIスケールをワールド行列に後掛け（平行移動も含め全体をスケール）
		Math::Matrix m = m_mWorld * uiScale;

		KdShaderManager::Instance().m_spriteShader.SetMatrix(m);
		if (m_bDrawTexture)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				m_specialAttackIconTexture,
				static_cast<int>(m_position.x), // 1920x1080基準の座標そのまま
				static_cast<int>(m_position.y),
				m_rect.width,
				m_rect.height,
				&m_rect,
				&m_color,
				{ 0.5f, 0.0f } // 下端基準で縮む
			);
		}
		KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	}
}
