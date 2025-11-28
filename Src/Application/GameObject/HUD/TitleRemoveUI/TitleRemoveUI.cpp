#include"TitleRemoveUI.h"
#include"Application/main.h"
#include "Application/Scene/SceneManager.h"

const uint32_t TitleRemoveUI::TypeID = KdGameObject::GenerateTypeID();

void TitleRemoveUI::Update()
{
	// 描画していないなら何もしない
	if (!m_bDrawTexture) return;

	// 現在のマウス座標（クライアント座標）を取得
	POINT pt{};
	if (!GetCursorPos(&pt)) return;

	// このアプリのメインウィンドウハンドルを取得
	// main.h などで管理しているウィンドウハンドルがあればそれを使う
	HWND hWnd = Application::Instance().GetWindowHandle(); // フレームワークにHWND取得APIがある前提
	if (!hWnd) return;

	ScreenToClient(hWnd, &pt);

	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);

	//マウス座標補正
	pt.x = static_cast<long>(vp.width) / 2;
	pt.y = static_cast<long>(vp.height) / 2;
	pt.y *= -1;

	// マウスがヒット矩形に入ったら当たり
	if (pt.x >= static_cast<long>(m_position.x - 100.0f) && pt.x <= static_cast<long>(m_position.x + 100.0f) &&
		pt.y >= static_cast<long>(m_position.y - 50.0f) && pt.y <= static_cast<long>(m_position.y + 50.0f))
	{
		m_hit = true;
	}
	else
	{
		m_hit = false;
	}


	KdDebugGUI::Instance().AddLog("Hit Status: %s\n", m_hit ? "Hit" : "Miss");

	// ヒット時に色を黄色へ、非ヒット時は元に戻す（ここでは白）
	if (m_hit)
	{
		m_color = Math::Color(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Title);
	}
	else
	{
		m_color = Math::Color(1.0f, 1.0f, 1.0f, 1.0f); // White
	}
}

void TitleRemoveUI::DrawSprite()
{
	// 現在のビューポートサイズ取得
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);

	// 伸張（Stretch）：XとYを個別にスケーリング（画面サイズにピッタリ）
	const float sx = vp.width / kRefW;
	const float sy = vp.height / kRefH;

	Math::Matrix uiScale = Math::Matrix::CreateScale(sx, sy, 1.0f);

	// UIスケールをワールド行列に後掛け（平行移動も含め全体をスケール）
	m_finalScaleMat = m_mWorld * uiScale;

	KdShaderManager::Instance().m_spriteShader.SetMatrix(m_finalScaleMat);
	if (m_bDrawTexture)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_texture,
			static_cast<int>(m_position.x), // 1920x1080基準の座標そのまま
			static_cast<int>(m_position.y),
			m_rect.width,
			m_rect.height,
			&m_rect,
			&m_color
		);
	}
	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
}