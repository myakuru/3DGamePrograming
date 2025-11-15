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
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_Attack/PlayerState_Attack.h"

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

	if (moveDir == Math::Vector3::Zero)
	{
		auto state = std::make_shared<PlayerState_RunEnd>();
		m_player->ChangeState(state);
		return;
	}

	// 押された瞬間
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_LButtonkeyInput = true; // 判定開始

	// 攻撃入力処理
	if (UpdateAttackInput<PlayerState_Attack>()) return;

	// 移動するときの入力処理
	if (UpdateMoveInput()) return;

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

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

bool PlayerState_Run::UpdateMoveInput()
{
	// W,S同時押し時は移動量リセット
	{
		if (KeyboardManager::GetInstance().IsKeyPressed('W') &&
			KeyboardManager::GetInstance().IsKeyPressed('S'))
		{
			// 移動量リセット
			m_player->SetIsMoving(Math::Vector3::Zero);
			m_player->SetMoveDirection(Math::Vector3::Zero);

			auto state = std::make_shared<PlayerState_RunEnd>();
			m_player->ChangeState(state);
			return true;
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
			return true;
		}
	}

	return false;
}
