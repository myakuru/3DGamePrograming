#include "ScoreBackBar.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"

const uint32_t ScoreBackBar::TypeID = KdGameObject::GenerateTypeID();

void ScoreBackBar::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;
	SelectDraw2DTexture::DrawSprite();
}

void ScoreBackBar::Update()
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (SceneManager::Instance().IsBossAppear())
	{
		if (m_position.y > m_dawnPos.y) m_position.y -= m_dawnTimer * deltaTime;
	}
}

void ScoreBackBar::ImGuiInspector()
{
	SelectDraw2DTexture::ImGuiInspector();
	ImGui::DragFloat("Dawn Timer", &m_dawnTimer);
	ImGui::DragFloat3("Dawn Position", &m_dawnPos.x);
}

void ScoreBackBar::JsonSave(nlohmann::json& _json) const
{
	SelectDraw2DTexture::JsonSave(_json);
	_json["DawnTimer"] = m_dawnTimer;
	_json["DawnPos"] = JSON_MANAGER.VectorToJson(m_dawnPos);
}

void ScoreBackBar::JsonInput(const nlohmann::json& _json)
{
	SelectDraw2DTexture::JsonInput(_json);
	if (_json.contains("DawnTimer")) m_dawnTimer = _json["DawnTimer"].get<float>();
	if (_json.contains("DawnPos")) m_dawnPos = JSON_MANAGER.JsonToVector(_json["DawnPos"]);
}
