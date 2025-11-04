#include "PlayerState_Attack4.h"
#include"../PlayerState_Sheathing-of-Katana/PlayerState_Sheathing-of-Katana.h"
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
#include"../../../BossEnemy/BossEnemy.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_Attack4::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("newAttack5");
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

	if (CharacterData::Instance().GetPlayerStatus().chargeCount < 3)
	{
		CharacterData::Instance().SetPlayerStatus().chargeCount++;
	}

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack4.WAV", false)->SetVolume(0.5f);
}

void PlayerState_Attack4::StateUpdate()
{
	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();

		m_maxAnimeTime = m_player->GetAnimator()->GetMaxAnimationTime();

		if (m_animeTime > m_maxAnimeTime)
		{
			KdDebugGUI::Instance().AddLog(U8("Attack4アニメ時間: %f"), m_animeTime);
			KdDebugGUI::Instance().AddLog("\n");
		}
		else
		{
			m_animeTime = m_maxAnimeTime;
		}
	}

	PlayerStateBase::StateUpdate();

	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// 0.5秒間当たり判定有効
	m_player->UpdateAttackCollision(7.0f, 1.0f, 7, m_maxAnimeTime, { 0.2f, 0.2f }, 0.3f);

	// 回避入力受付
	{
		if (KeyboardManager::GetInstance().IsKeyPressed('W') && KeyboardManager::GetInstance().IsKeyJustPressed(VK_RBUTTON))
		{
			auto sheath = std::make_shared<PlayerState_BackWordAvoid>();
			m_player->ChangeState(sheath);
			return;
		}

		if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_RBUTTON))
		{
			auto sheath = std::make_shared<PlayerState_ForwardAvoid>();
			m_player->ChangeState(sheath);
			return;
		}
	}

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		m_LButtonkeyInput = true;
	}

	// Eキー先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed('E'))
	{
		if (CharacterData::Instance().GetPlayerStatus().skillPoint >= 30)
		{
			m_EButtonkeyInput = true;
			CharacterData::Instance().SetPlayerStatus().skillPoint -= 30;
		}
	}

	if (KeyboardManager::GetInstance().IsKeyJustPressed('Q'))
	{
		if (CharacterData::Instance().GetPlayerStatus().specialPoint == CharacterData::Instance().GetPlayerStatus().specialPointMax)
		{
			CharacterData::Instance().SetPlayerStatus().specialPoint = 0;
			auto specialAttackState = std::make_shared<PlayerState_SpecialAttackCutIn>();
			m_player->ChangeState(specialAttackState);
			return;
		}
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


		if (m_EButtonkeyInput)
		{
			m_EButtonkeyInput = false;
			auto runState = std::make_shared<PlayerState_Skill>();
			m_player->ChangeState(runState);
			return;
		}

		// コンボ受付
		if (m_LButtonkeyInput || KeyboardManager::GetInstance().IsKeyPressed(VK_LBUTTON))
		{
			// 90%以降で受付
			if (m_animeTime < 0.9f) return;

			const float kLongPressThreshold = 0.5f; // 長押し閾値
			const bool isPressed = KeyboardManager::GetInstance().IsKeyPressed(VK_LBUTTON);
			const bool isJustPressed = KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON);
			const float lDuration = isPressed ? KeyboardManager::GetInstance().GetKeyPressDuration(VK_LBUTTON) : 0.0f;

			// 現在のチャージ残数
			int& chargeCount = CharacterData::Instance().SetPlayerStatus().chargeCount;

			// 1) 先行入力を最優先で消費してAttack1へ
			if (m_LButtonkeyInput)
			{
				m_LButtonkeyInput = false;
				auto state = std::make_shared<PlayerState_Attack1>();
				m_player->ChangeState(state);
				return;
			}

			// 2) チャージが0以下で長押し中の場合
			if (chargeCount <= 0 && isPressed)
			{
				auto state = std::make_shared<PlayerState_SheathKatana>();
				m_player->ChangeState(state);
				return;
			}

			// 3) チャージが残っている場合のみ、長押しでFullChargeへ
			if (chargeCount > 0 && isPressed && lDuration >= kLongPressThreshold)
			{
				chargeCount = std::max(0, chargeCount - 1); // 念のため下限ガード
				auto state = std::make_shared<PlayerState_FullCharge>();
				m_player->ChangeState(state);
				return;
			}

			// 4) 受付内の新規押下でもAttack1へ
			if (isJustPressed)
			{
				auto state = std::make_shared<PlayerState_Attack1>();
				m_player->ChangeState(state);
				return;
			}
		}
		else if (m_player->GetAnimator()->IsAnimationEnd())
		{
			auto sheath = std::make_shared<PlayerState_SheathKatana>();
			m_player->ChangeState(sheath);
			return;
		}
	}
}

void PlayerState_Attack4::StateEnd()
{
	PlayerStateBase::StateEnd();
	// カメラの位置を変更
	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		if (auto bossEnemy = m_bossEnemy.lock(); bossEnemy)
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
