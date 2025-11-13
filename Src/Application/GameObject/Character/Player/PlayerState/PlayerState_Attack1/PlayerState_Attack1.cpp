#include "PlayerState_Attack1.h"
#include"../PlayerState_Attack2/PlayerState_Attack2.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../../main.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
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
	auto anime = m_player->GetAnimeModel()->GetAnimation("Attack1");
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
	// アニメーション時間の取得
	m_animeTime = m_player->GetAnimator()->GetPlayProgress();

	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	// 0.5秒間当たり判定有効
	m_player->UpdateAttackCollision(3.0f, 1.0f, 1, 0.1f, { 0.2f, 0.2f }, 0.2f);
	
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

		if (auto playreCamera = m_player->GetPlayerCamera().lock(); playreCamera)
		{
			playreCamera->StartShake({ m_player->GetCameraShakePower()}, m_player->GetCameraShakeTime());
		}

		// 攻撃入力受付
		if (m_animeTime >= 0.7f)
		{
			// ため攻撃入力処理
			if (UpdateChargeAttackInput()) return;

			// 攻撃入力処理
			if (UpdateAttackInput<PlayerState_Attack2>()) return;

			// アニメーション終了後の遷移処理
			if (UpdateSheathKatanaInput()) return;
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
