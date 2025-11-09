#include "PlayerState_Idle.h"
#include"../../../CharacterBase.h"
#include"../../../../../main.h"

#include"../../../../../Scene/SceneManager.h"

#include"../PlayerState_Run/PlayerState_Run.h"
#include"../PlayerState_Attack/PlayerState_Attack.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"
#include"../PlayerState_ChargeAttack/PlayerState_ChargeAttack.h"
#include"../PlayerState_Hit/PlayerState_Hit.h"
#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"../PlayerState_FullCharge/PlayerState_FullCharge.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"

void PlayerState_Idle::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Idle");
	m_player->GetAnimator()->SetAnimation(anime);

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		if(auto bossEnemy = m_bossEnemy.lock(); bossEnemy)
		{
			camera->SetTargetLookAt(m_cameraBossTargetOffset);
		}
		else
		{
			camera->SetTargetLookAt(m_cameraTargetOffset);
		}
	}
	m_isKeyPressing = false;
}

void PlayerState_Idle::StateUpdate()
{
	m_player->SetAnimeSpeed(60.0f);

	UpdateUnsheathed();

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

	// 前方回避
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_RBUTTON))
	{
		auto fowardAvoidState = std::make_shared<PlayerState_ForwardAvoid>();
		m_player->ChangeState(fowardAvoidState);
		return;
	}

	// Eスキル
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

	// 押された瞬間
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		m_isKeyPressing = true; // 判定開始
	}

	{
		// =========================
		// 左ボタン押下 長押し / 短押し判定
		// =========================

		const float kShortPressMin = 0.1f; // 短押し有効開始
		const float kLongPressThreshold = 0.5f; // 長押し閾値(これ以上で長押しアクション)

		float lDuration = KeyboardManager::GetInstance().GetKeyPressDuration(VK_LBUTTON);

		// 1) 先行入力を最優先で消費してAttackへ
		if (m_isKeyPressing)
		{
			m_isKeyPressing = false;
			auto state = std::make_shared<PlayerState_Attack>();
			m_player->ChangeState(state);
			return;
		}

		// Chargeカウントがあり、長押し状態へ移行
		if (m_playerData.GetPlayerStatus().chargeCount > 0 && m_isKeyPressing && lDuration >= kLongPressThreshold)
		{
			m_isKeyPressing = false;

			m_playerData.SetPlayerStatus().chargeCount--;

			auto avoidFast = std::make_shared<PlayerState_FullCharge>();
			m_player->ChangeState(avoidFast);
			return;
		}

		// 短押し判定
		if (m_isKeyPressing)
		{
			if (lDuration >= kShortPressMin && lDuration < kLongPressThreshold)
			{
				auto backAvoid = std::make_shared<PlayerState_Attack>();
				m_player->ChangeState(backAvoid);
				m_isKeyPressing = false;
				return;
			}

			// 0.1秒未満なら何もしない
			m_isKeyPressing = false;
		}
	}

	// 移動量リセット
	m_player->SetIsMoving(Math::Vector3::Zero);

}

void PlayerState_Idle::StateEnd()
{
	PlayerStateBase::StateEnd();
}

