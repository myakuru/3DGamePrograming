#include "SpecialAttackUI.h"
#include"../../../Scene/SceneManager.h"
#include"../../Character/Player/Player.h"

const uint32_t SpecialAttackUI::TypeID = KdGameObject::GenerateTypeID();

void SpecialAttackUI::Init()
{
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameUI/SpecialMove.png");
	m_specialAttackIconTexture = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameUI/onSpecialMove.png");

	SceneManager::Instance().GetObjectWeakPtr(m_player);

	m_showSpecialAttackUI = false;
}

void SpecialAttackUI::Update()
{
	if (auto player = m_player.lock(); player)
	{
		if (player->GetUseSpecial())
		{
			m_showSpecialAttackUI = true;
		}
		else
		{
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
				nullptr,
				&m_color
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
				nullptr,
				&m_color
			);
		}
		KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	}
}
