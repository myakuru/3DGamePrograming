#include "AetheriusEnemyConfig.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/RedEnemy.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyStateBase.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"

// 攻撃系ステート
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Attack/EnemyState_Attack.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Attack1/EnemyState_Attack1.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Attack2/EnemyState_Attack2.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Attack3/EnemyState_Attack3.h"

// 行動系ステート
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Idle/EnemyState_Idle.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Run/EnemyState_Run.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Walk_Left/EnemyState_Walk_Left.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Walk_Right/EnemyState_Walk_Right.h"

// 被弾・死亡系ステート
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Hit/EnemyState_Hit.h"
#include "Application/GameObject/Character/EnemyBase/AetheriusEnemy/EnemyState/EnemyState_Death/EnemyState_Death.h"

void AetheriusEnemyConfig::InGuiInspector()
{
	ImGui::Separator();
	DrawStateComboImGui();
	ImGui::Separator();

	if (m_selectedStateIndex >= 0 && m_selectedStateIndex < (int)m_states.size())
	{
		ImGui::Text("Parameters: %s", m_currentStateName.c_str());
		ImGui::Indent();
		m_states[m_selectedStateIndex]->ExposeParametersImGui();
		ImGui::Unindent();
	}
}

void AetheriusEnemyConfig::JsonInput(const nlohmann::json& js)
{
	for (const auto& st : m_states) st->LoadParametersJson(js);
}

void AetheriusEnemyConfig::JsonSave() const
{
	nlohmann::json cfg = nlohmann::json::object();
	for (auto& st : m_states) st->SaveParametersJson(cfg);
	JSON_MANAGER.JsonSerialize(cfg, "Json/AetheriusEnemyConfig/AetheriusEnemyConfig");
}

void AetheriusEnemyConfig::CreateStates()
{
	m_states.clear();
	m_stateNames.clear();

	// 追加ステート数に合わせて予約（10種類）
	m_states.reserve(10);
	m_stateNames.reserve(10);

	// ===== 攻撃系 =====
	m_states.emplace_back(std::make_unique<EnemyState_Attack>());   m_stateNames.emplace_back("Attack");
	m_states.emplace_back(std::make_unique<EnemyState_Attack1>());  m_stateNames.emplace_back("Attack1");
	m_states.emplace_back(std::make_unique<EnemyState_Attack2>());  m_stateNames.emplace_back("Attack2");
	m_states.emplace_back(std::make_unique<EnemyState_Attack3>());   m_stateNames.emplace_back("Attack3");

	// ===== 行動系 =====
	m_states.emplace_back(std::make_unique<EnemyState_Idle>());     m_stateNames.emplace_back("Idle");
	m_states.emplace_back(std::make_unique<EnemyState_Run>());      m_stateNames.emplace_back("Run");
	m_states.emplace_back(std::make_unique<EnemyState_Walk_Left>());  m_stateNames.emplace_back("Walk_Left");
	m_states.emplace_back(std::make_unique<EnemyState_Walk_Right>()); m_stateNames.emplace_back("Walk_Right");

	// ===== 被弾 / 死亡系 =====
	m_states.emplace_back(std::make_unique<EnemyState_Hit>());      m_stateNames.emplace_back("Hit");
	m_states.emplace_back(std::make_unique<EnemyState_Death>());    m_stateNames.emplace_back("Death");

	// 初期選択を未選択に戻す
	m_selectedStateIndex = -1;
	m_currentStateName = "None";
}

void AetheriusEnemyConfig::DrawStateComboImGui()
{
	// 同期
	if (m_selectedStateIndex >= 0 &&
		m_selectedStateIndex < (int)m_stateNames.size() &&
		(m_currentStateName == "None" || m_currentStateName != m_stateNames[m_selectedStateIndex]))
	{
		m_currentStateName = m_stateNames[m_selectedStateIndex];
	}

	const char* preview = m_currentStateName.c_str();

	if (ImGui::BeginCombo("##EnemyStateCombo", preview))
	{
		for (int i = 0; i < (int)m_stateNames.size(); ++i)
		{
			const std::string& name = m_stateNames[i];
			bool selected = (m_currentStateName == name);
			if (ImGui::Selectable(name.c_str(), selected))
			{
				m_currentStateName = name;
				m_selectedStateIndex = i;
			}
			if (selected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void AetheriusEnemyConfig::ApplyPrototypeParametersTo(EnemyStateBase& runtime)
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