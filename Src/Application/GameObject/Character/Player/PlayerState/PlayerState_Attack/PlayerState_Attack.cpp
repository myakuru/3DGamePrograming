#include "PlayerState_Attack.h"
#include"../../../../../main.h"
#include"../PlayerState_Attack1/PlayerState_Attack1.h"
#include"../PlayerState_FullCharge/PlayerState_FullCharge.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/SwordFlash/SwordFlash.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"


void PlayerState_Attack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Attack");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	PlayerStateBase::StateStart();

	m_attackParam = m_player->GetPlayerConfig().GetAttack2Param();
	m_attackParam.m_dashTimer = 0.0f;

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// 攻撃時はtrueにする
	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		katana->SetNowAttackState(true);
	}

	m_LButtonkeyInput = false;		// 次段コンボ予約フラグ初期化
	m_time = 0.0f;					// 当たり判定用

	SceneManager::Instance().GetObjectWeakPtr(m_slashEffect);
	m_player->SetAnimeSpeed(70.0f);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack.wav", false)->SetVolume(1.0f);

	m_LButtonkeyInput = false;
}

void PlayerState_Attack::StateUpdate()
{

	// アニメーション時間の取得
	m_animeTime = m_player->GetAnimator()->GetPlayProgress();

	float deltaTime = Application::Instance().GetDeltaTime();

	// 当たり判定有効時間: 最初の0.5秒のみ
	m_player->UpdateAttackCollision(3.0f, 1.0f, 1, 0.1f, { 0.2f, 0.0f }, 0.3f, 0.0f, 0.4f);


	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

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

	// 先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		m_LButtonkeyInput = true;
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
			m_EButtonkeyInput = true;
			m_playerData.SetPlayerStatus().skillPoint -= 30;
		}
	}

	// 先行ダッシュ処理
	if (m_time < 0.2f)
	{
		const float dashSpeed = 0.7f;
		m_player->SetIsMoving(m_attackDirection * dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		// エフェクト再生・移動停止
		if (auto effect = m_slashEffect.lock(); effect)
		{
			effect->SetPlayEffect(true);
		}

		m_player->SetIsMoving(Math::Vector3::Zero);

		if (m_EButtonkeyInput)
		{
			m_EButtonkeyInput = false;
			auto runState = std::make_shared<PlayerState_Skill>();
			m_player->ChangeState(runState);
			return;
		}

		// コンボ受付
		if (m_LButtonkeyInput)
		{
			// 70%以降で受付
			if (m_animeTime < 0.7f) return;

			const float kLongPressThreshold = 0.1f; // 長押し閾値
			const bool isPressed = KeyboardManager::GetInstance().IsKeyPressed(VK_LBUTTON);
			const bool isJustPressed = KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON);
			const float lDuration = isPressed ? KeyboardManager::GetInstance().GetKeyPressDuration(VK_LBUTTON) : 0.0f;

			// 1) 先行入力を最優先で消費してAttack1へ
			if (m_LButtonkeyInput)
			{
				m_LButtonkeyInput = false;
				auto state = std::make_shared<PlayerState_Attack1>();
				m_player->ChangeState(state);
				return;
			}

			// チャージが0以下で長押し中の場合
			/*if (m_player->GetStatus().GetPlayerStatus().chargeCount <= 0 && isPressed)
			{
				auto state = std::make_shared<PlayerState_SheathKatana>();
				m_player->ChangeState(state);
				return;
			}*/

			// チャージが残っている場合のみ、長押しでFullChargeへ
			if (KeyboardManager::GetInstance().GetKeyPressDuration(VK_LBUTTON) >= kLongPressThreshold)
			{
				auto state = std::make_shared<PlayerState_FullCharge>();
				m_player->ChangeState(state);
				return;
			}

			// 4) 受付内の新規押下でもAttack1へ
			if (isJustPressed)
			{
				m_LButtonkeyInput = false;
				auto state = std::make_shared<PlayerState_Attack1>();
				m_player->ChangeState(state);
				return;
			}
		}
		else if (m_player->GetAnimator()->IsAnimationEnd())
		{
			if (auto swordEffect = m_slashEffect.lock(); swordEffect)
			{
				if (!swordEffect->IsEffectPlaying())
				{
					auto sheath = std::make_shared<PlayerState_SheathKatana>();
					m_player->ChangeState(sheath);
					return;
				}
			}
		}
	}

	PlayerStateBase::StateUpdate();

	// カタナ関連
	auto katana = m_player->GetKatana().lock();
	if (!katana) return;

	katana->SetShowTrail(true);
	UpdateKatanaPos();
}

void PlayerState_Attack::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto swordEffect = m_slashEffect.lock(); swordEffect)
	{
		swordEffect->SetPlayEffect(false);
	}
}