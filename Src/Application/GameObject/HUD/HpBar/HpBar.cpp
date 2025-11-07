#include "HpBar.h"
#include"../../Utility/Time.h"
#include"../../../Scene/SceneManager.h"
#include"../../../Data/CharacterData/CharacterData.h"
#include"../../Character/Player/Player.h"

const uint32_t HpBar::TypeID = KdGameObject::GenerateTypeID();

void HpBar::Init()
{
	SelectDraw2DTexture::Init();

	// 初期は満タン表示
	m_hpRate = 1.0f;
	m_rect.width = static_cast<float>(1500.0f * m_hpRate);

	SceneManager::Instance().GetObjectWeakPtr(m_player);
}

void HpBar::DrawSprite()
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

void HpBar::Update()
{
	// 目標のHP割合を算出（0～1にクランプ）
	
	if (auto playerStatus = m_player.lock(); playerStatus)
	{
		float denom = playerStatus->GetStatus().GetCharacterData().maxHp > 0 ? static_cast<float>(playerStatus->GetStatus().GetCharacterData().maxHp) : 1.0f;

		float targetHpRate = static_cast<float>(playerStatus->GetStatus().GetCharacterData().hp) / denom;
		
		if (targetHpRate < 0.0f) targetHpRate = 0.0f;
		if (targetHpRate > 1.0f) targetHpRate = 1.0f;


		// 経過時間から補間係数を計算（秒ベース）
		const float now = Time::Instance().GetElapsedTime();
		static float prev = now;
		float dt = now - prev;
		if (dt < 0.0f) dt = 0.0f;
		prev = now;

		// 追従速度
		const float followSpeed = 6.0f;
		// 補間係数
		float t = followSpeed * dt;
		if (t > 1.0f) t = 1.0f;
		if (t < 0.0f) t = 0.0f;

		// 現在表示値を目標へ徐々に近づける
		m_hpRate += (targetHpRate - m_hpRate) * t;

		// 表示幅を更新
		m_rect.width = static_cast<float>(1500.0f * m_hpRate);

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
}

void HpBar::ImGuiInspector()
{
	SelectDraw2DTexture::ImGuiInspector();
}