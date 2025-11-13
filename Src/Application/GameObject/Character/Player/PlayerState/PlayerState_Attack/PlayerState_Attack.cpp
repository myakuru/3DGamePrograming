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

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// 先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		m_LButtonkeyInput = true;
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