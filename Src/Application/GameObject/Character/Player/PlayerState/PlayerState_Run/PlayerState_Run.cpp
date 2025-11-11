#include "PlayerState_Run.h"
#include"../../../CharacterBase.h"
#include"../PlayerState_RunEnd/PlayerState_RunEnd.h"
#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../PlayerState_Hit/PlayerState_Hit.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../PlayerState_FowardAvoidFast/PlayerState_FowardAvoidFast.h"
#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_Run::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Run");
	m_player->GetAnimator()->SetAnimation(anime);
	m_player->SetAnimeSpeed(70.0f);
	PlayerStateBase::StateStart();

	if (!m_runSound)
	{
		m_runSound = KdAudioManager::Instance().Play("Asset/Sound/Player/Run.wav", true);
	}
}

void PlayerState_Run::StateUpdate()
{
	UpdateUnsheathed();
	m_player->UpdateMoveDirectionFromInput();

	Math::Vector3 moveDir = m_player->GetMoveDirection();

	if (KeyboardManager::GetInstance().IsKeyPressed('W') &&
		KeyboardManager::GetInstance().IsKeyPressed('S'))
	{
		// 移動量リセット
		m_player->SetIsMoving(Math::Vector3::Zero);
		m_player->SetMoveDirection(Math::Vector3::Zero);

		auto state = std::make_shared<PlayerState_RunEnd>();
		m_player->ChangeState(state);
		return;
	}

	// D,A同時押し時は移動量リセット
	if (KeyboardManager::GetInstance().IsKeyPressed('D') &&
		KeyboardManager::GetInstance().IsKeyPressed('A'))
	{
		// 移動量リセット
		m_player->SetIsMoving(Math::Vector3::Zero);
		m_player->SetMoveDirection(Math::Vector3::Zero);

		auto state = std::make_shared<PlayerState_RunEnd>();
		m_player->ChangeState(state);
		return;
	}

	if (moveDir == Math::Vector3::Zero)
	{
		auto state = std::make_shared<PlayerState_RunEnd>();
		m_player->ChangeState(state);
		return;
	}

	// =========================
	// 右ボタン押下 長押し / 短押し判定
	// =========================
	const float kShortPressMin = 0.1f; // 短押し有効開始
	const float kLongPressThreshold = 0.2f; // 長押し閾値(これ以上で長押しアクション)

	float rDuration = KeyboardManager::GetInstance().GetKeyPressDuration(VK_RBUTTON);

	// 押された瞬間
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_RBUTTON))
	{
		m_isKeyPressing = true; // 判定開始
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

	// 長押し判定
	if (m_isKeyPressing && rDuration >= kLongPressThreshold && !KeyboardManager::GetInstance().IsKeyJustReleased(VK_RBUTTON))
	{
		m_isKeyPressing = false;
		auto avoidFast = std::make_shared<PlayerState_FowardAvoidFast>();
		m_player->ChangeState(avoidFast);
		return;
	}

	// 短押し判定
	if (m_isKeyPressing && KeyboardManager::GetInstance().IsKeyJustReleased(VK_RBUTTON))
	{
		if (rDuration >= kShortPressMin && rDuration < kLongPressThreshold)
		{
			auto backAvoid = std::make_shared<PlayerState_BackWordAvoid>();
			m_player->ChangeState(backAvoid);
			m_isKeyPressing = false;
			return;
		}

		// 0.1秒未満なら何もしない
		m_isKeyPressing = false;
	}

	// 方向更新
	m_player->UpdateQuaternion(moveDir);
	m_player->SetIsMoving(moveDir);
}

void PlayerState_Run::StateEnd()
{
	PlayerStateBase::StateEnd();
	if (m_runSound)
	{
		m_runSound->Stop();     // 公開APIで停止
		m_runSound.reset();
	}
}
