#include "PlayerState_RunEnd.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include "../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_Attack/PlayerState_Attack.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"../../../../../main.h"

void PlayerState_RunEnd::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("RunEnd");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);
}

void PlayerState_RunEnd::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	// 移動方向を取得
	{
		m_attackDirection = m_player->GetMovement();

		if (m_player->GetMovement() != Math::Vector3::Zero)
		{
			m_attackDirection.y = 0.0f;
			m_attackDirection.Normalize();
			m_player->UpdateQuaternionDirect(m_attackDirection);
		}
	}

	// 押された瞬間
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_lButtonKeyInput = true; // 判定開始

	// 攻撃入力処理
	if (UpdateAttackInput<PlayerState_Attack>()) return;

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// Eスキル入力処理
	if (UpdateESkillInput()) return;

	// 移動するときの入力処理
	if (UpdateMoveInput()) return;

	// アニメーション終了でIdleへ
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(state);
		return;
	}

	UpdateUnsheathed();

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		m_player->SetIsMoving(Math::Vector3::Zero);
	}
}

void PlayerState_RunEnd::StateEnd()
{
	PlayerStateBase::StateEnd();
}

bool PlayerState_RunEnd::UpdateMoveInput()
{
	// WキーとSキーが同時押しされたらIdle状態に戻る
	if (const auto& keyboardManager = KeyboardManager::GetInstance();
		keyboardManager.IsKeyPressed('W') &&
		keyboardManager.IsKeyPressed('S'))
	{
		if (m_player->GetAnimator()->IsAnimationEnd())
		{
			auto state = std::make_shared<PlayerState_Idle>();
			m_player->ChangeState(state);
			return true;
		}
	}
	else if (keyboardManager.IsKeyPressed('D') &&
		keyboardManager.IsKeyPressed('A'))
	{
		if (m_player->GetAnimator()->IsAnimationEnd())
		{
			auto state = std::make_shared<PlayerState_Idle>();
			m_player->ChangeState(state);
			return true;
		}
	}
	else if (keyboardManager.IsKeyPressed('W') ||
		keyboardManager.IsKeyPressed('A') ||
		keyboardManager.IsKeyPressed('S') ||
		keyboardManager.IsKeyPressed('D'))
	{

		auto state = std::make_shared<PlayerState_Run>();
		m_player->ChangeState(state);
		return true;

	}

	return false;
}

void PlayerState_RunEnd::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_RunEnd));
	const auto& p = static_cast<const PlayerState_RunEnd&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.dashSpeedTime = p.m_stateParameter.dashSpeedTime;
	m_stateParameter.dashSpeed = p.m_stateParameter.dashSpeed;
}

void PlayerState_RunEnd::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::DragFloat(U8("ダッシュ速度時間"), &m_stateParameter.dashSpeedTime);
	ImGui::DragFloat(U8("ダッシュ速度"), &m_stateParameter.dashSpeed);
}

void PlayerState_RunEnd::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_RunEnd")) return;
	const auto& stateNode = js["PlayerState_RunEnd"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("dashSpeedTime")) m_stateParameter.dashSpeedTime = playerNode["dashSpeedTime"].get<float>();
		if (playerNode.contains("dashSpeed")) m_stateParameter.dashSpeed = playerNode["dashSpeed"].get<float>();
	}
}

void PlayerState_RunEnd::SaveParametersJson(nlohmann::json& js) const
{
	if (!js.contains("PlayerState_RunEnd")) js["PlayerState_RunEnd"] = nlohmann::json::object();
	auto& stateNode = js["PlayerState_RunEnd"];

	stateNode["Player"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["Player"]["animationSpeed"] = m_stateParameter.animationSpeed;
	stateNode["Player"]["dashSpeedTime"] = m_stateParameter.dashSpeedTime;
	stateNode["Player"]["dashSpeed"] = m_stateParameter.dashSpeed;
}
