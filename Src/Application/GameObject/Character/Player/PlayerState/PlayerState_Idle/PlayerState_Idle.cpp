#include "PlayerState_Idle.h"
#include"Application/main.h"
#include"Application/Scene/SceneManager.h"

#include"Application/GameObject/Weapon/Katana/Katana.h"
#include"Application/GameObject/Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"

#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"

#include"Application/GameObject/Character/Player/PlayerState/PlayerState_Run/PlayerState_Run.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_Attack/PlayerState_Attack.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_Hit/PlayerState_Hit.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_Skill/PlayerState_Skill.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_RunEnd/PlayerState_RunEnd.h"

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

	m_lButtonKeyInput = false;
}

void PlayerState_Idle::StateUpdate()
{
	m_player->SetAnimeSpeed(60.0f);

	UpdateUnsheathed();

	if (KeyboardManager::GetInstance().IsKeyPressed('W') &&
		KeyboardManager::GetInstance().IsKeyPressed('S'))
	{
		// 移動量リセット
		m_player->SetIsMoving(Math::Vector3::Zero);
		m_player->SetMoveDirection(Math::Vector3::Zero);
		return;
	}
	// D,Aキーが同時に押されている場合、移動しない
	if (KeyboardManager::GetInstance().IsKeyPressed('D') &&
		KeyboardManager::GetInstance().IsKeyPressed('A'))
	{
		// 移動量リセット
		m_player->SetIsMoving(Math::Vector3::Zero);
		m_player->SetMoveDirection(Math::Vector3::Zero);
		return;
	}

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

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// Eスキル入力処理
	if (UpdateESkillInput()) return;

	// 押された瞬間
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_lButtonKeyInput = true; // 判定開始

	// 攻撃入力処理
	if (UpdateAttackInput<PlayerState_Attack>()) return;

	// 移動量リセット
	m_player->SetIsMoving(Math::Vector3::Zero);

}

void PlayerState_Idle::StateEnd()
{
	PlayerStateBase::StateEnd();
}

