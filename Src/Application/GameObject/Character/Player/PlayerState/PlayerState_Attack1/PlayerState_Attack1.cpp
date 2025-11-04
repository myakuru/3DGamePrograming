#include "PlayerState_Attack1.h"
#include"../PlayerState_Attack2/PlayerState_Attack2.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../../main.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../PlayerState_Sheathing-of-Katana/PlayerState_Sheathing-of-Katana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../../../../Effect/EffekseerEffect/Rotation/Rotation.h"

#include"../PlayerState_FullCharge/PlayerState_FullCharge.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_Attack1::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("newAttack2");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);
	

	PlayerStateBase::StateStart();

	m_attackParam = m_player->GetPlayerConfig().GetAttack1Param();

	m_attackParam.m_dashTimer = 0.0f;

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// 攻撃時はtrueにする
	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		katana->SetNowAttackState(true);
	}

	m_effectOnce = false;

	m_LButtonkeyInput = false;

	// エフェクトの取得
	SceneManager::Instance().GetObjectWeakPtr(m_effect);

	m_player->SetAnimeSpeed(70.0f);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack1.wav", false)->SetVolume(0.5f);
}

void PlayerState_Attack1::StateUpdate()
{
	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();

		m_maxAnimeTime = m_player->GetAnimator()->GetMaxAnimationTime();

		if (m_animeTime > m_maxAnimeTime)
		{
			KdDebugGUI::Instance().AddLog(U8("Attack1アニメ時間: %f"), m_animeTime);
			KdDebugGUI::Instance().AddLog("\n");
		}
		else
		{
			m_animeTime = m_maxAnimeTime;
		}
	}

	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// 0.5秒間当たり判定有効
	m_player->UpdateAttackCollision(3.0f, 1.0f, 1, 0.1f, { 0.2f, 0.2f }, 0.2f);
	
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

	// Eキー先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed('E'))
	{
		if (CharacterData::Instance().GetPlayerStatus().skillPoint >= 30)
		{
			m_EButtonkeyInput = true;
			CharacterData::Instance().SetPlayerStatus().skillPoint -= 30;
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

		// エフェクトの再生
		if (auto effet = m_effect.lock(); effet)
		{
			effet->SetPlayEffect(true);
		}

		if (m_EButtonkeyInput)
		{
			m_EButtonkeyInput = false;
			auto runState = std::make_shared<PlayerState_Skill>();
			m_player->ChangeState(runState);
			return;
		}

		if (auto playreCamera = m_player->GetPlayerCamera().lock(); playreCamera)
		{
			playreCamera->StartShake({ m_player->GetCameraShakePower()}, m_player->GetCameraShakeTime());
		}

		// コンボ受付
		if (m_LButtonkeyInput || KeyboardManager::GetInstance().IsKeyPressed(VK_LBUTTON))
		{
			// 70%以降で受付
			if (m_animeTime < 0.7f) return;

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
				auto state = std::make_shared<PlayerState_Attack2>();
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
				auto state = std::make_shared<PlayerState_Attack2>();
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


	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		// 剣の軌跡の表示
		katana->SetShowTrail(true);
		// 現在攻撃中フラグを立てる
		katana->SetNowAttackState(true);
		// 手と鞘の位置を更新
		UpdateKatanaPos();
	}

}

void PlayerState_Attack1::StateEnd()
{
	PlayerStateBase::StateEnd();

	// エフェクトがあったらフラグをfalseにする
	if(auto effect = m_effect.lock(); effect)
	{
		effect->SetPlayEffect(false);
		effect->StopEffect();
	}

	// カタナの軌跡を消す
	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		katana->SetNowAttackState(false);
	}
}
