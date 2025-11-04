#include "UpDownField.h"
#include "../../../Scene/SceneManager.h"
#include"../../../main.h"

const uint32_t UpDownField::TypeID = KdGameObject::GenerateTypeID();

void UpDownField::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.SetDitherEnable(false);
	SelectDraw3dModel::DrawLit();
}

void UpDownField::DrawGrayScale()
{
	if (!SceneManager::Instance().GetDrawGrayScale()) return;

	SelectDraw3dModel::DrawGrayScale();
}

void UpDownField::Update()
{
	float deltaTime = Application::Instance().GetDeltaTime();
	m_time += deltaTime * m_speed;

	// sinの[-1,1] → [0,1] に正規化して補間係数tを作る
	const float time = 0.5f * (std::sin(m_time * 2.0f) + 1.0f);
	m_position.y = std::lerp(m_min, m_max, time);
}

void UpDownField::ImGuiInspector()
{
	SelectDraw3dModel::ImGuiInspector();
	ImGui::InputFloat("Min", &m_min);
	ImGui::InputFloat("Max", &m_max);
	ImGui::InputFloat("Speed", &m_speed);
}

void UpDownField::JsonSave(nlohmann::json& _json) const
{
	SelectDraw3dModel::JsonSave(_json);
	_json["min"] = m_min;
	_json["max"] = m_max;
	_json["speed"] = m_speed;
}

void UpDownField::JsonInput(const nlohmann::json& _json)
{
	SelectDraw3dModel::JsonInput(_json);
	if (_json.contains("min")) m_min = _json["min"].get<float>();
	if (_json.contains("max")) m_max = _json["max"].get<float>();
	if (_json.contains("speed")) m_speed = _json["speed"].get<float>();
}
