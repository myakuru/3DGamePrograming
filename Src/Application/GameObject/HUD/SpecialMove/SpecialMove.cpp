#include "SpecialMove.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"
#include"../../../Data/CharacterData/CharacterData.h"
#include"../../Character/Player/Player.h"

const uint32_t SpecialMove::TypeID = KdGameObject::GenerateTypeID();

void SpecialMove::Init()
{
	m_displayTime = 1000;
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/Time/SpecialMovePTR.png");

	m_limitPointFlag = false;

	SceneManager::Instance().GetObjectWeakPtr(m_player);

}

void SpecialMove::Update()
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();
	m_timer += deltaTime;

	if (auto playerStatus = m_player.lock(); playerStatus)
	{
		m_displayTime = playerStatus->GetStatus().GetPlayerStatus().specialPoint;

		// 満タン演出は >= にして上限超えでも演出を維持
		if (playerStatus->GetStatus().GetPlayerStatus().specialPoint >= playerStatus->GetStatus().GetPlayerStatus().specialPointMax)
		{
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
		}
	}
}

void SpecialMove::DrawSprite()
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

	// 桁ごとの幅（1桁50px、間隔500pxで描画しているので、1桁分の描画幅は500px）
	const int digitWidth = 300;
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
