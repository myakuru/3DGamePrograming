#include "PlayerConfig.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState.h"

// 攻撃系
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Attack/PlayerState_Attack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Attack1/PlayerState_Attack1.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Attack2/PlayerState_Attack2.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Attack3/PlayerState_Attack3.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Attack4/PlayerState_Attack4.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidAttack/PlayerState_AvoidAttack.h"

// スキル・特殊攻撃
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Skill/PlayerState_Skill.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttack/PlayerState_SpecialAttack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttack1/PlayerState_SpecialAttack1.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

// 溜め攻撃・チャージ段階
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevel1/PlayerState_ChargeLevel1.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevel2/PlayerState_ChargeLevel2.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelMax/PlayerState_ChargeLevelMax.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeAttackMax/PlayerState_ChargeAttackMax.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeAttackMax1/PlayerState_ChargeAttackMax1.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeAttackMax2/PlayerState_ChargeAttackMax2.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeAttackMax3/PlayerState_ChargeAttackMax3.h"

// 回避関連
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_FowardAvoidFast/PlayerState_FowardAvoidFast.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_JustAvoidAttack/PlayerState_JustAvoidAttack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_JustAvoidAttack_end/PlayerState_JustAvoidAttack_end.h"

// Hit
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Hit/PlayerState_Hit.h"

// その他
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Idle/PlayerState_Idle.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_Run/PlayerState_Run.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_RunEnd/PlayerState_RunEnd.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"

#include"MyFramework/Manager/JsonManager/JsonManager.h"
#include"Application/main.h"

void PlayerConfig::InGuiInspector()
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

void PlayerConfig::DrawStateComboImGui()
{
	// 初期同期
	if (m_selectedStateIndex >= 0 &&
		m_selectedStateIndex < (int)m_stateNames.size() &&
		(m_currentStateName == "None" || m_currentStateName != m_stateNames[m_selectedStateIndex]))
	{
		m_currentStateName = m_stateNames[m_selectedStateIndex];
	}

	const char* preview = m_currentStateName.data();

	//## で表示名とID分離
	if (ImGui::BeginCombo("##PlayerStateCombo", preview))
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

void PlayerConfig::JsonInput(const nlohmann::json& js)
{
	for (const auto& st : m_states) st->LoadParametersJson(js);
}

void PlayerConfig::JsonSave() const
{
	nlohmann::json cfg = nlohmann::json::object();
	for (auto& st : m_states) st->SaveParametersJson(cfg);
	JSON_MANAGER.JsonSerialize(cfg, "Json/PlayerConfig/PlayerConfig"); // ここで書き出す
}

void PlayerConfig::CreateStates()
{
	m_states.clear();
	m_stateNames.clear();
	m_states.reserve(24);
	m_stateNames.reserve(24);

	// 攻撃
	m_states.emplace_back(std::make_unique<PlayerState_Attack>());          m_stateNames.emplace_back("Attack");
	m_states.emplace_back(std::make_unique<PlayerState_Attack1>());         m_stateNames.emplace_back("Attack1");
	m_states.emplace_back(std::make_unique<PlayerState_Attack2>());         m_stateNames.emplace_back("Attack2");
	m_states.emplace_back(std::make_unique<PlayerState_Attack3>());         m_stateNames.emplace_back("Attack3");
	m_states.emplace_back(std::make_unique<PlayerState_Attack4>());         m_stateNames.emplace_back("Attack4");
	m_states.emplace_back(std::make_unique<PlayerState_AvoidAttack>());    m_stateNames.emplace_back("AvoidAttack");

	// スキル・特殊
	m_states.emplace_back(std::make_unique<PlayerState_Skill>());           m_stateNames.emplace_back("Skill");
	m_states.emplace_back(std::make_unique<PlayerState_SpecialAttack>());   m_stateNames.emplace_back("SpecialAttack");
	m_states.emplace_back(std::make_unique<PlayerState_SpecialAttack1>());  m_stateNames.emplace_back("SpecialAttack1");
	m_states.emplace_back(std::make_unique<PlayerState_SpecialAttackCutIn>()); m_stateNames.emplace_back("SpecialAttackCutIn");

	// チャージ
	m_states.emplace_back(std::make_unique<PlayerState_ChargeLevel0>());    m_stateNames.emplace_back("ChargeLevel0");
	m_states.emplace_back(std::make_unique<PlayerState_ChargeLevel1>());    m_stateNames.emplace_back("ChargeLevel1");
	m_states.emplace_back(std::make_unique<PlayerState_ChargeLevel2>());    m_stateNames.emplace_back("ChargeLevel2");
	m_states.emplace_back(std::make_unique<PlayerState_ChargeLevelMax>());  m_stateNames.emplace_back("ChargeLevelMax");
	m_states.emplace_back(std::make_unique<PlayerState_ChargeAttackMax>()); m_stateNames.emplace_back("ChargeAttackMax");
	m_states.emplace_back(std::make_unique<PlayerState_ChargeAttackMax1>()); m_stateNames.emplace_back("ChargeAttackMax1");
	m_states.emplace_back(std::make_unique<PlayerState_ChargeAttackMax2>()); m_stateNames.emplace_back("ChargeAttackMax2");
	m_states.emplace_back(std::make_unique<PlayerState_ChargeAttackMax3>()); m_stateNames.emplace_back("ChargeAttackMax3");

	// 回避
	m_states.emplace_back(std::make_unique<PlayerState_FowardAvoid>());     m_stateNames.emplace_back("FowardAvoid");
	m_states.emplace_back(std::make_unique<PlayerState_FowardAvoidFast>()); m_stateNames.emplace_back("FowardAvoidFast");
	m_states.emplace_back(std::make_unique<PlayerState_BackWordAvoid>());   m_stateNames.emplace_back("BackWordAvoid");
	m_states.emplace_back(std::make_unique<PlayerState_JustAvoidAttack>()); m_stateNames.emplace_back("JustAvoidAttack");
	m_states.emplace_back(std::make_unique<PlayerState_JustAvoidAttack_end>()); m_stateNames.emplace_back("JustAvoidAttack_end");

	// Hit
	m_states.emplace_back(std::make_unique<PlayerState_Hit>());             m_stateNames.emplace_back("Hit");
	// その他
	m_states.emplace_back(std::make_unique<PlayerState_Idle>());            m_stateNames.emplace_back("Idle");
	m_states.emplace_back(std::make_unique<PlayerState_Run>());             m_stateNames.emplace_back("Run");
	m_states.emplace_back(std::make_unique<PlayerState_RunEnd>());          m_stateNames.emplace_back("RunEnd");
	m_states.emplace_back(std::make_unique<PlayerState_SheathKatana>());   m_stateNames.emplace_back("SheathKatana");

	m_selectedStateIndex = m_states.empty() ? -1 : 0;
	m_currentStateName = (m_selectedStateIndex >= 0) ? m_stateNames[m_selectedStateIndex] : "None";
}

// Player側で使用する変数に、ステートのパラメータを適用する
void PlayerConfig::ApplyPrototypeParametersTo(PlayerStateBase& runtime)
{
	for (const auto& proto : m_states)
	{
		if (typeid(*proto) == typeid(runtime))
		{
			// ここでコピーしてあげる
			runtime.ApplyFromConfig(*proto);
			break;
		}
	}
}