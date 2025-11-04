#include "Scoring.h"
#include"../../Utility/Time.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"

const uint32_t Scoring::TypeID = KdGameObject::GenerateTypeID();

void Scoring::Init()
{
	SelectDraw2DTexture::Init();
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameUI/Score.png");
}

void Scoring::Update()
{
	// 残り秒数を取得
	float timeLeft = Time::Instance().GetCountdownTimeLeft();

	float delataTime = Application::Instance().GetUnscaledDeltaTime();

	m_displayTime = static_cast<int>(timeLeft);

	if (m_displayTime >= 60)
	{
		SceneManager::Instance().SetScore(0); //S
		m_srcRect = { 0,0,50,50 };
		m_color = { 1.0f, 0.43f, 0.04f, 1.0f };
	}
	else if (m_displayTime >= 30)
	{
		SceneManager::Instance().SetScore(1); //A
		m_srcRect = { 50,0,50,50 };
		m_color = { 0.71f, 0.24f, 1.0f, 1.0f };
	}
	else if(m_displayTime >= 10)
	{
		SceneManager::Instance().SetScore(2); //B
		m_srcRect = { 100,0,50,50 };
		m_color = { 0.19f, 0.62f, 1.0f, 1.0f };
	}
	else
	{
		SceneManager::Instance().SetScore(3); //X
	}

	if (SceneManager::Instance().IsBossAppear())
	{
		if (m_position.y > m_dawnPos.y) m_position.y -= m_dawnTimer * delataTime;
	}

}

void Scoring::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;

	// 現在のビューポートサイズ取得
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);
	// 伸張（Stretch）：XとYを個別にスケーリング（画面サイズにピッタリ）
	const float sx = vp.width / kRefW;
	const float sy = vp.height / kRefH;
	// 等方（Fit）にしたい場合は以下を使う（必要なら切替）
	//const float s  = std::min(sx, sy);
	// Math::Matrix uiScale = Math::Matrix::CreateScale(s, s, 1.0f);
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
			&m_srcRect,
			&m_color
		);
	}
	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
}

void Scoring::ImGuiInspector()
{
	SelectDraw2DTexture::ImGuiInspector();

	ImGui::DragFloat("Dawn Timer", &m_dawnTimer);
	ImGui::DragFloat3("Dawn Position", &m_dawnPos.x);
}

void Scoring::JsonSave(nlohmann::json& _json) const
{
	SelectDraw2DTexture::JsonSave(_json);
	_json["DawnTimer"] = m_dawnTimer;
	_json["DawnPos"] = JSON_MANAGER.VectorToJson(m_dawnPos);
}

void Scoring::JsonInput(const nlohmann::json& _json)
{
	SelectDraw2DTexture::JsonInput(_json);
	if (_json.contains("DawnTimer")) m_dawnTimer = _json["DawnTimer"].get<float>();
	if (_json.contains("DawnPos")) m_dawnPos = JSON_MANAGER.JsonToVector(_json["DawnPos"]);
}
