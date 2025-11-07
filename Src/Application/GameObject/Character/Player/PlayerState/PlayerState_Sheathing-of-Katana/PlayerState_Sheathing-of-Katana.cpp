#include "PlayerState_Sheathing-of-Katana.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Attack/PlayerState_Attack.h"
#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include "../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_FullCharge/PlayerState_FullCharge.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_SheathKatana::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("SheathKatana");
	m_player->GetAnimator()->SetAnimation(anime, 0.9f, false);
	

	PlayerStateBase::StateStart();

	m_player->m_onceEffect = false;

	if(!m_SheathKatanaSound) m_SheathKatanaSound = KdAudioManager::Instance().Play("Asset/Sound/Player/SheathKatana.wav", false);

	m_LButtonkeyInput = false;
}

void PlayerState_SheathKatana::StateUpdate()
{
	m_player->SetAnimeSpeed(120.0f);

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON) || KeyboardManager::GetInstance().IsKeyPressed(VK_LBUTTON))
	{
		m_LButtonkeyInput = true;
	}

	if (m_LButtonkeyInput)
	{
		const float kLongPressThreshold = 0.1f; // 長押し閾値
		const bool isPressed = KeyboardManager::GetInstance().IsKeyPressed(VK_LBUTTON);
		const bool isJustPressed = KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON);
		const float lDuration = isPressed ? KeyboardManager::GetInstance().GetKeyPressDuration(VK_LBUTTON) : 0.0f;

		// 現在のチャージ残数
		int& chargeCount = m_playerData.SetPlayerStatus().chargeCount;

		// 1) 先行入力を最優先で消費してAttack1へ
		if (m_LButtonkeyInput)
		{
			m_LButtonkeyInput = false;
			auto state = std::make_shared<PlayerState_Attack>();
			m_player->ChangeState(state);
			return;
		}

		// 2) チャージが0以下で長押し中の場合
		if (chargeCount <= 0 && isPressed)
		{
			if (m_player->GetAnimator()->IsAnimationEnd())
			{
				auto idleState = std::make_shared<PlayerState_Idle>();
				m_player->ChangeState(idleState);
				return;
			}
		}

		// 3) チャージが残っている場合のみ、長押しでFullChargeへ
		if (chargeCount > 0 && isPressed && lDuration >= kLongPressThreshold)
		{
			auto state = std::make_shared<PlayerState_FullCharge>();
			m_player->ChangeState(state);
			return;
		}

		// 4) 受付内の新規押下でもAttack1へ
		if (isJustPressed)
		{
			auto state = std::make_shared<PlayerState_Attack>();
			m_player->ChangeState(state);
			return;
		}
	}

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_RBUTTON))
	{
		auto attackState = std::make_shared<PlayerState_ForwardAvoid>();
		m_player->ChangeState(attackState);
		return;
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

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto idleState = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(idleState);
		return;
	}

	// カタナの取得
	auto katana = m_player->GetKatana().lock();
	if (!katana) return;
	
	float time = m_player->GetAnimator()->GetTime();

	katana->SetShowTrail(false);

	if (time >= 20.0f)
	{
		katana->SetNowAttackState(true);
		UpdateKatanaPos();
	}
	else
	{
		katana->SetNowAttackState(false);
		UpdateUnsheathed();
	}

}

void PlayerState_SheathKatana::StateEnd()
{
	PlayerStateBase::StateEnd();

	if(m_SheathKatanaSound)
	{
		m_SheathKatanaSound->Stop();
		m_SheathKatanaSound.reset();
	}
}
