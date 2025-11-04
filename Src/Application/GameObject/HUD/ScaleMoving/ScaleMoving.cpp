#include "ScaleMoving.h"
#include"../../Utility/Time.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"

const uint32_t ScaleMoving::TypeID = KdGameObject::GenerateTypeID();


void ScaleMoving::Update()
{
	float timeLeft = Time::Instance().GetCountdownTimeLeft();
	const float maxTime = 120.0f; // タイマーの最大値

	// 0.0f ~ 1.0f にクランプ
	m_time = std::clamp(timeLeft / maxTime, 0.0f, 1.0f);

	// 可視幅（ピクセル）
	const float visibleWidthF = std::clamp(m_fullWidth * m_time, 0.0f, m_fullWidth);
	const int   visibleWidth = static_cast<int>(std::round(visibleWidthF));

	// 右端を基準にソース矩形を左からカットする
	// m_fullWidth はテクスチャ(バー)の最大幅と一致している前提
	m_rect.x = static_cast<int>(m_fullWidth) - visibleWidth; // 左へスライド
	m_rect.width = visibleWidth;
	// m_rect.y, m_rect.height は固定（丸みを保つために高さは不変）

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (SceneManager::Instance().IsBossAppear())
	{
		if (m_position.y > m_dawnPos.y) m_position.y -= m_dawnTimer * deltaTime;
	}

	// 行列更新
	m_mWorld = Math::Matrix::CreateScale(m_scale);
	m_mWorld *= Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_degree.y),
		DirectX::XMConvertToRadians(m_degree.x),
		DirectX::XMConvertToRadians(m_degree.z)
	);
	m_mWorld.Translation(m_position);
}

void ScaleMoving::DrawSprite()
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
		static_cast<int>(m_position.x), // 右端のスクリーン座標
		static_cast<int>(m_position.y),
		m_rect.width,
		m_rect.height,
		&m_rect,
		&m_color,
		{ 1.0f, 0.5f } // 右端基準で縮む＆移動
	);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
}

void ScaleMoving::ImGuiInspector()
{
	SelectDraw2DTexture::ImGuiInspector();
	ImGui::DragFloat("Dawn Timer", &m_dawnTimer);
	ImGui::DragFloat3("Dawn Position", &m_dawnPos.x);
}

void ScaleMoving::JsonSave(nlohmann::json& _json) const
{
	SelectDraw2DTexture::JsonSave(_json);
	_json["DawnTimer"] = m_dawnTimer;
	_json["DawnPos"] = JSON_MANAGER.VectorToJson(m_dawnPos);
}

void ScaleMoving::JsonInput(const nlohmann::json& _json)
{
	SelectDraw2DTexture::JsonInput(_json);
	if (_json.contains("DawnTimer")) m_dawnTimer = _json["DawnTimer"].get<float>();
	if (_json.contains("DawnPos")) m_dawnPos = JSON_MANAGER.JsonToVector(_json["DawnPos"]);
}
