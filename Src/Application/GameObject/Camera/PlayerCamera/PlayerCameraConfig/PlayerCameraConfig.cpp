#include "PlayerCameraConfig.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"
#include "Application/main.h"

#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_IntroCamera/PlayerCameraState_IntroCamera.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_LookPlayer/PlayerCameraState_LookPlayer.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_WinnerCamera_1st/PlayerCameraState_WinnerCamera_1st.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_WinnerCamera_2nd/PlayerCameraState_WinnerCamera_2nd.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_WinnerCamera_3rd/PlayerCameraState_WinnerCamera_3rd.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCameraState/PlayerCameraState_WinnerCamera_Final/PlayerCameraState_WinnerCamera_Final.h"

void PlayerCameraConfig::InGuiInspector()
{
	ImGui::Separator();
	DrawStateComboImGui();
	ImGui::Separator();

	if (m_selectedStateIndex >= 0 && m_selectedStateIndex < (int)m_states.size())
	{
		ImGui::Text("Parameters: %s", m_currentStateName.data());
		ImGui::Indent();
		m_states[m_selectedStateIndex]->ExposeParametersImGui();
		ImGui::Unindent();
	}
}

void PlayerCameraConfig::JsonInput(const nlohmann::json& js)
{
	for (const auto& st : m_states) st->LoadParametersJson(js);
}

void PlayerCameraConfig::JsonSave() const
{
	nlohmann::json cfg = nlohmann::json::object();
	for (auto& st : m_states) st->SaveParametersJson(cfg);
	JSON_MANAGER.JsonSerialize(cfg, "Json/PlayerCameraConfig/PlayerCameraConfig");
}

void PlayerCameraConfig::CreateStates()
{
	m_states.clear();
	m_stateNames.clear();

	// 追加ステート数（13種類）
	m_states.reserve(6);
	m_stateNames.reserve(6);

	// ===== イントロカメラ =====
	m_states.emplace_back(std::make_unique<PlayerCameraState_IntroCamera>());        m_stateNames.emplace_back("IntroCamera");

	// ===== プレイヤーを追従するカメラ =====
	m_states.emplace_back(std::make_unique<PlayerCameraState_LookPlayer>());        m_stateNames.emplace_back("LookPlayer");

	// ===== 勝利演出のカメラ =====
	m_states.emplace_back(std::make_unique<PlayerCameraState_WinnerCamera_1st>());        m_stateNames.emplace_back("WinnerCamera_1st");
	m_states.emplace_back(std::make_unique<PlayerCameraState_WinnerCamera_2nd>());        m_stateNames.emplace_back("WinnerCamera_2nd");
	m_states.emplace_back(std::make_unique<PlayerCameraState_WinnerCamera_3rd>());        m_stateNames.emplace_back("WinnerCamera_3rd");
	m_states.emplace_back(std::make_unique<PlayerCameraState_WinnerCamera_Final>());      m_stateNames.emplace_back("WinnerCamera_Final");
}

void PlayerCameraConfig::ApplyPrototypeParametersTo(PlayerCameraState& runtime)
{
	for (const auto& proto : m_states)
	{
		if (typeid(*proto) == typeid(runtime))
		{
			runtime.ApplyFromConfig(*proto);
			break;
		}
	}
}

void PlayerCameraConfig::DrawStateComboImGui()
{
	if (m_selectedStateIndex >= 0 &&
		m_selectedStateIndex < (int)m_stateNames.size() &&
		(m_currentStateName == "None" || m_currentStateName != m_stateNames[m_selectedStateIndex]))
	{
		m_currentStateName = m_stateNames[m_selectedStateIndex];
	}

	const char* preview = m_currentStateName.data();

	if (ImGui::BeginCombo("##PlayerCameraStateCombo", preview))
	{
		for (int i = 0; i < (int)m_stateNames.size(); ++i)
		{
			const std::string& name = m_stateNames[i];
			bool selected = (m_currentStateName == name);
			if (ImGui::Selectable(name.data(), selected))
			{
				m_currentStateName = name;
				m_selectedStateIndex = i;
			}
			if (selected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}
