#include "BossEnemyUI.h"
#include"../../../Scene/SceneManager.h"
#include"../../Character/BossEnemy/BossEnemy.h"

const uint32_t BossEnemyUI::TypeID = KdGameObject::GenerateTypeID();

void BossEnemyUI::Init()
{
	SelectDraw2DTexture::Init();

	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/EnemyUI/BossHpBar.png");

	// 初期は満タン表示
	m_hpRate = 1.0f;
	m_rect.width = static_cast<float>(1500.0f * m_hpRate);

	m_srcRect = { 0, 0, 1500, 300 };
}

void BossEnemyUI::DrawSprite()
{
	if (!SceneManager::Instance().IsBossAppear()) return; // ボス出現していなければ描画しない

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
		static_cast<int>(m_position.x),
		static_cast<int>(m_position.y),
		m_rect.width,          // 表示幅（可変）
		m_rect.height,
		&m_srcRect,            // Updateで右側から切り出し
		&m_color,
		{ 1.0f, 0.5f }         // 右端基準（drawX補正は不要）
	);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
}

void BossEnemyUI::Update()
{
	if (!SceneManager::Instance().IsBossAppear()) return; // ボス出現していなければ描画しない

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	if (auto bossEnemy = m_bossEnemy.lock())
	{
		// 目標のHP割合を算出（0～1にクランプ）
		m_hpRate = bossEnemy->GetStatus().maxHp > 0
			? std::clamp(static_cast<float>(bossEnemy->GetStatus().hp) / static_cast<float>(bossEnemy->GetStatus().maxHp), 0.0f, 1.0f)
			: 1.0f;

		// 表示幅（可変）
		const long visibleW = std::clamp<long>(static_cast<long>(std::round(1500 * m_hpRate)), 0, 1500);

		// 描画先のサイズ
		m_rect.width = visibleW;

		// テクスチャの右側から切り出す（左から消える）
		m_srcRect.x = 1500 - visibleW;
		m_srcRect.y = 0;
		m_srcRect.width = visibleW;
	}

	// 行列更新
	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll(m_degree.y, m_degree.x, m_degree.z);
	m_mWorld.Translation(m_position);
}
