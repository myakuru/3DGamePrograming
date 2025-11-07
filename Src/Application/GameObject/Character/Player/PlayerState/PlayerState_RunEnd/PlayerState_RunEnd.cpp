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
	m_player->GetAnimator()->SetAnimation(anime, 0.1f, false);
	m_player->SetAnimeSpeed(70.0f);
}

void PlayerState_RunEnd::StateUpdate()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		auto attackState = std::make_shared<PlayerState_Attack>();
		m_player->ChangeState(attackState);
		return;
	}

	// Eキー先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed('E'))
	{
		if (m_playerData.GetPlayerStatus().skillPoint >= 30)
		{
			m_playerData.SetPlayerStatus().skillPoint -= 30;
			auto state = std::make_shared<PlayerState_Skill>();
			m_player->ChangeState(state);
			return;
		}
	}

	if (KeyboardManager::GetInstance().IsKeyJustPressed('Q'))
	{
		if (m_playerData.GetPlayerStatus().specialPoint == m_playerData.GetPlayerStatus().specialPointMax)
		{
			m_playerData.SetPlayerStatus().specialPoint = 0;
			auto specialAttackState = std::make_shared<PlayerState_SpecialAttackCutIn>();
			m_player->ChangeState(specialAttackState);
			return;
		}
	}

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_RBUTTON))
	{
		auto attackState = std::make_shared<PlayerState_BackWordAvoid>();
		m_player->ChangeState(attackState);
		return;
	}

	// キー入力から移動方向を更新
	m_player->UpdateMoveDirectionFromInput();

	// キー入力されたらRun状態に戻る
	if (m_player->GetMoveDirection() != Math::Vector3::Zero)
	{
		auto state = std::make_shared<PlayerState_Run>();
		m_player->ChangeState(state);
		return;
	}

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		m_attackDirection = m_player->GetMovement();
		m_attackDirection.y = 0.0f;
		m_attackDirection.Normalize();
		m_player->UpdateQuaternionDirect(m_attackDirection);
	}

	// キー入力から移動方向を更新
	m_player->UpdateMoveDirectionFromInput();

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(state);
		return;
	}

	UpdateUnsheathed();

	if (m_time < 0.5f)
	{
		const float dashSpeed = 0.3f;
		m_player->SetIsMoving(m_attackDirection * dashSpeed);
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
