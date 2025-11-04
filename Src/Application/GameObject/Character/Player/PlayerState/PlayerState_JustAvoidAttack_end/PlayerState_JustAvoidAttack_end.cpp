#include "PlayerState_JustAvoidAttack_end.h"
#include"../../../../../main.h"
#include"../PlayerState_FullCharge/PlayerState_FullCharge.h"
#include"../PlayerState_Sheathing-of-Katana/PlayerState_Sheathing-of-Katana.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/SwordFlash/SwordFlash.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../../../../Effect/EffekseerEffect/JustAvoidAttackEffect/JustAvoidAttackEffect.h"
#include"../../../BossEnemy/BossEnemy.h"

void PlayerState_JustAvoidAttack_end::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("JustAvoidAttack");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);
	PlayerStateBase::StateStart();
	m_time = 0.0f;

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(100.0f);

	SceneManager::Instance().GetObjectWeakPtr(m_justAvoidAttackEffect);

	if (auto effect = m_justAvoidAttackEffect.lock(); effect)
	{
		effect->SetPlayEffect(true);
	}

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	// Chargeカウントを1増やす（最大3まで）
	if (CharacterData::Instance().GetPlayerStatus().chargeCount < 3)
	{
		CharacterData::Instance().SetPlayerStatus().chargeCount++;
	}
}

void PlayerState_JustAvoidAttack_end::StateUpdate()
{

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_SheathKatana>();
		m_player->ChangeState(state);
		return;
	}

	// 当たり判定有効時間: 最初の0.5秒のみ
	m_player->UpdateAttackCollision(8.0f, 1.0f, 5, 0.1f, { 0.3f, 0.3f }, 0.3f);

	UpdateKatanaPos();

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	// 目標到達 → 停止して向きのみ合わせる
	m_player->SetIsMoving(Math::Vector3::Zero);

}

void PlayerState_JustAvoidAttack_end::StateEnd()
{
	PlayerStateBase::StateEnd();

	// 敵との当たり判定を有効化（押し出し処理を元に戻す）
	m_player->SetAtkPlayer(false);

	// 無敵解除
	m_player->SetInvincible(false);

	// スローモーション解除（ここを終点にする）
	Application::Instance().SetFpsScale(1.f);
	SceneManager::Instance().SetDrawGrayScale(false);

	// ジャスト回避フラグを戻す
	m_player->SetJustAvoidSuccess(false);


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

	if (auto effect = m_justAvoidAttackEffect.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	// ゲームのメインサウンドのピッチを元に戻す
	if (auto bgm = SceneManager::Instance().GetGameSound())
	{
		bgm->SetPitch(0.0f);
	}
}