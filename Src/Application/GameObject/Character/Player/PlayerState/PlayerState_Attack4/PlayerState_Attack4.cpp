#include "PlayerState_Attack4.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../main.h"
#include"../PlayerState_Attack1/PlayerState_Attack1.h"
#include"../PlayerState_FullCharge/PlayerState_FullCharge.h"
#include"../../../../../Scene/SceneManager.h"

#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Effect/EffekseerEffect/GroundFreezes/GroundFreezes.h"
#include"../../../../Effect/EffekseerEffect/Rotation/Rotation.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_Attack4::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Attack4");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	PlayerStateBase::StateStart();

	// 攻撃時はtrueにする
	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		katana->SetNowAttackState(true);
	}

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	SceneManager::Instance().GetObjectWeakPtr(m_groundFreezes);
	SceneManager::Instance().GetObjectWeakPtr(m_rotation);
	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	m_time = 0.0f;
	m_LButtonkeyInput = false;

	m_player->SetAnimeSpeed(70.0f);

	if (m_playerData.GetPlayerStatus().chargeCount < 3)
	{
		m_playerData.SetPlayerStatus().chargeCount++;
	}

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack4.WAV", false)->SetVolume(0.5f);
}

void PlayerState_Attack4::StateUpdate()
{
	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();

		m_maxAnimeTime = m_player->GetAnimator()->GetMaxAnimationTime();
	}

	PlayerStateBase::StateUpdate();

	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// 0.5秒間当たり判定有効
	m_player->UpdateAttackCollision(7.0f, 1.0f, 7, m_maxAnimeTime, { 0.2f, 0.2f }, 0.3f);

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// 先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		m_LButtonkeyInput = true;
	}

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	UpdateKatanaPos();

	if (m_attackParam.m_dashTimer < 0.2f)
	{
		float dashSpeed = 1.0f;
		m_player->SetIsMoving(m_attackDirection * dashSpeed);
		m_attackParam.m_dashTimer += deltaTime;
	}
	else
	{
		// 移動を止める

		m_player->SetIsMoving(Math::Vector3::Zero);

		if (auto effect = m_groundFreezes.lock(); effect)
		{
			effect->SetPlayEffect(true);
		}

		if (auto effect = m_rotation.lock(); effect)
		{
			effect->SetPlayEffect(true);
		}


		// 攻撃入力受付
		if (m_animeTime >= 0.7f)
		{
			// ため攻撃入力処理
			if (UpdateChargeAttackInput()) return;

			// 攻撃入力処理
			if (UpdateAttackInput<PlayerState_Attack1>()) return;

			// アニメーション終了後の遷移処理
			if (UpdateSheathKatanaInput()) return;
		}
	}
}

void PlayerState_Attack4::StateEnd()
{
	PlayerStateBase::StateEnd();
	// カメラの位置を変更
	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		if (auto bossEnemy = m_bossEnemy.lock())
		{
			camera->SetTargetLookAt(m_cameraBossTargetOffset);
		}
		else
		{
			camera->SetTargetLookAt(m_cameraTargetOffset);
		}
	}

	if (auto effect = m_groundFreezes.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	if (auto effect = m_rotation.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	m_player->SetIsMoving(Math::Vector3::Zero);
}
