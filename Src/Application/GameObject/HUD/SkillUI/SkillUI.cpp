#include "SkillUI.h"
#include"../../../Scene/SceneManager.h"
#include"../../Character/Player/Player.h"

const uint32_t SkillUI::TypeID = KdGameObject::GenerateTypeID();

void SkillUI::Init()
{
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameUI/nonSkile.png");
	m_skillIconTexture = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameUI/Skile.png");

	SceneManager::Instance().GetObjectWeakPtr(m_player);

}

void SkillUI::Update()
{
	if (auto player = m_player.lock(); player)
	{
		if (player->GetUseSkill())
		{
			m_showSkillUI = true;
		}
		else
		{
			m_showSkillUI = false;
		}
	}
}

void SkillUI::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;

	if (!m_showSkillUI)
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
				m_skillIconTexture,
				static_cast<int>(m_position.x), // 1920x1080基準の座標そのまま
				static_cast<int>(m_position.y),
				nullptr,
				&m_color
			);
		}
		KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	}
}
