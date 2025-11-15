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
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_LButtonkeyInput = true; // 判定開始

	// 攻撃入力処理
	if (UpdateAttackInput<PlayerState_Attack>()) return;

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// キーが押されたらRunステートへ
	if (KeyboardManager::GetInstance().IsKeyPressed('W') ||
		KeyboardManager::GetInstance().IsKeyPressed('A') ||
		KeyboardManager::GetInstance().IsKeyPressed('S') ||
		KeyboardManager::GetInstance().IsKeyPressed('D'))
	{
		auto spRunState = std::make_shared<PlayerState_Run>();
		m_player->ChangeState(spRunState);
		return;
	}

	// WキーとSキーが同時押しされたらIdle状態に戻る
	if (KeyboardManager::GetInstance().IsKeyPressed('W') &&
		KeyboardManager::GetInstance().IsKeyPressed('S'))
	{
		if(m_player->GetAnimator()->IsAnimationEnd())
		{
			auto state = std::make_shared<PlayerState_Idle>();
			m_player->ChangeState(state);
			return;
		}
	}

	// DキーとAキーが同時押しされたらIdle状態に戻る
	if (KeyboardManager::GetInstance().IsKeyPressed('D') &&
		KeyboardManager::GetInstance().IsKeyPressed('A'))
	{
		if (m_player->GetAnimator()->IsAnimationEnd())
		{
			auto state = std::make_shared<PlayerState_Idle>();
			m_player->ChangeState(state);
			return;
		}
	}

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
