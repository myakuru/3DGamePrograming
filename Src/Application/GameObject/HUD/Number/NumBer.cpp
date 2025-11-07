#include "NumBer.h"
#include"../../../Scene/SceneManager.h"
#include"../../../Data/CharacterData/CharacterData.h"
#include"../../Character/Player/Player.h"

const uint32_t NumBer::TypeID = KdGameObject::GenerateTypeID();

void NumBer::Init()
{
	m_displayTime = 0;
	m_isIncreasing = false;
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/Time/Hp.png");

	m_displayTime = 10000;

	SceneManager::Instance().GetObjectWeakPtr<Player>(m_player);

}

void NumBer::Update()
{
	if (auto playerStatus = m_player.lock(); playerStatus)
	{
		m_displayTime = playerStatus->GetStatus().GetCharacterData().hp;
	}
}

void NumBer::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;

	// 現在のビューポートサイズ取得
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);

	// 伸張（Stretch）：XとYを個別にスケーリング（画面サイズにピッタリ）
	const float sx = vp.width / kRefW;
	const float sy = vp.height / kRefH;

	Math::Matrix uiScale = Math::Matrix::CreateScale(sx, sy, 1.0f);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(m_mWorld * uiScale);

	// m_displayTimeの値を文字列化
	std::string numStr = std::to_string(m_displayTime);

	int baseX = static_cast<int>(m_position.x);
	int baseY = static_cast<int>(m_position.y);

	// 桁ごとの幅
	const int digitWidth = 250;
	const int totalWidth = static_cast<int>(numStr.size()) * digitWidth;

	// 中心寄せのため、描画開始位置を調整
	int startX = baseX - totalWidth / 2;

	// 各桁を左から順に描画
	for (size_t i = 0; i < numStr.size(); ++i)
	{
		int n = numStr[i] - '0'; // 文字→数字
		int texIndex = n;
		Math::Rectangle srcRect = { 50 * texIndex, 0, 50, 50 };
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_texture,
			startX + digitWidth * static_cast<int>(i),
			baseY,
			&srcRect,
			&m_color
		);
	}

	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
}
