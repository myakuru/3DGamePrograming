#include "BossEnemyConfig.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState.h"
#include "Application/main.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"

// ===== 攻撃系 =====
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_Attack_L/BossEnemyState_Attack_L.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_Attack_R/BossEnemyState_Attack_R.h"

// ===== チャージ系 =====
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_ChargeStart/BossEnemyState_ChargeStart.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_ChrgeLoop/BossEnemyState_ChrgeLoop.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_ChargeEnd/BossEnemyState_ChargeEnd.h"

// ===== 行動 / 回避 / 登場 =====
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_Dodge/BossEnemyState_Dodge.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_Enter/BossEnemyState_Enter.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_Idle/BossEnemyState_Idle.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_Run/BossEnemyState_Run.h"

// ===== ヒット系 =====
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_Hit/BossEnemyState_Hit.h"

// ===== 水攻撃系 =====
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_WaterAttack/BossEnemyState_WaterAttack.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_WaterAttack_end/BossEnemyState_WaterAttack_end.h"
#include "Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemyState/BossEnemyState_WaterFall/BossEnemyState_WaterFall.h"

void BossEnemyConfig::InGuiInspector()
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

void BossEnemyConfig::JsonInput(const nlohmann::json& js)
{
	for (const auto& st : m_states) st->LoadParametersJson(js);
}

void BossEnemyConfig::JsonSave() const
{
	nlohmann::json cfg = nlohmann::json::object();
	for (auto& st : m_states) st->SaveParametersJson(cfg);
	JSON_MANAGER.JsonSerialize(cfg, "Json/BossEnemyConfig/BossEnemyConfig");
}

void BossEnemyConfig::CreateStates()
{
	m_states.clear();
	m_stateNames.clear();

	// 追加ステート数（13種類）
	m_states.reserve(13);
	m_stateNames.reserve(13);

	// ===== 攻撃系 =====
	m_states.emplace_back(std::make_unique<BossEnemyState_Attack_L>());        m_stateNames.emplace_back("Attack_L");
	m_states.emplace_back(std::make_unique<BossEnemyState_Attack_R>());        m_stateNames.emplace_back("Attack_R");

	// ===== チャージ系 =====
	m_states.emplace_back(std::make_unique<BossEnemyState_ChargeStart>());     m_stateNames.emplace_back("ChargeStart");
	m_states.emplace_back(std::make_unique<BossEnemyState_ChrgeLoop>());       m_stateNames.emplace_back("ChrgeLoop");   // 綴りはフォルダ/クラス名に合わせています
	m_states.emplace_back(std::make_unique<BossEnemyState_ChargeEnd>());       m_stateNames.emplace_back("ChargeEnd");

	// ===== 行動 / 回避 / 登場 =====
	m_states.emplace_back(std::make_unique<BossEnemyState_Dodge>());           m_stateNames.emplace_back("Dodge");
	m_states.emplace_back(std::make_unique<BossEnemyState_Enter>());           m_stateNames.emplace_back("Enter");
	m_states.emplace_back(std::make_unique<BossEnemyState_Idle>());            m_stateNames.emplace_back("Idle");
	m_states.emplace_back(std::make_unique<BossEnemyState_Run>());             m_stateNames.emplace_back("Run");

	// ===== ヒット =====
	m_states.emplace_back(std::make_unique<BossEnemyState_Hit>());             m_stateNames.emplace_back("Hit");

	// ===== 水攻撃系 =====
	m_states.emplace_back(std::make_unique<BossEnemyState_WaterAttack>());     m_stateNames.emplace_back("WaterAttack");
	m_states.emplace_back(std::make_unique<BossEnemyState_WaterAttack_end>()); m_stateNames.emplace_back("WaterAttack_end");
	m_states.emplace_back(std::make_unique<BossEnemyState_WaterFall>());       m_stateNames.emplace_back("WaterFall");

	m_selectedStateIndex = -1;
	m_currentStateName = "None";
}

void BossEnemyConfig::DrawStateComboImGui()
{
	if (m_selectedStateIndex >= 0 &&
		m_selectedStateIndex < (int)m_stateNames.size() &&
		(m_currentStateName == "None" || m_currentStateName != m_stateNames[m_selectedStateIndex]))
	{
		m_currentStateName = m_stateNames[m_selectedStateIndex];
	}

	const char* preview = m_currentStateName.c_str();

	if (ImGui::BeginCombo("##BossEnemyStateCombo", preview))
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

void BossEnemyConfig::ApplyPrototypeParametersTo(BossEnemyStateBase& runtime)
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