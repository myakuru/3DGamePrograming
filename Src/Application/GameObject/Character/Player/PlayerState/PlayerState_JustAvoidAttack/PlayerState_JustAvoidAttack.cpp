#include "PlayerState_JustAvoidAttack.h"
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

#include"../PlayerState_JustAvoidAttack_end/PlayerState_JustAvoidAttack_end.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../../../../Effect/EffekseerEffect/JustAvoidAttackEffect1/JustAvoidAttackEffect1.h"
#include"../../../BossEnemy/BossEnemy.h" // 追加

void PlayerState_JustAvoidAttack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Eskill");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	PlayerStateBase::StateStart();

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// 攻撃時はtrueにする
	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		katana->SetNowAttackState(true);
	}

	m_LButtonkeyInput = false;      // 次段コンボ予約フラグ初期化
	m_player->SetAnimeSpeed(70.0f);

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	m_player->SetInvincible(true);

	m_player->SetJustAvoidSuccess(true);

	// Just回避演出を確実に適用（冪等化）
	{
		const auto& justCfg = m_player->GetPlayerConfig().GetJustAvoidParam();
		Application::Instance().SetFpsScale(justCfg.m_slowMoScale);
		SceneManager::Instance().SetDrawGrayScale(justCfg.m_useGrayScale);
	}

	SceneManager::Instance().GetObjectWeakPtr(m_justAvoidAttackEffect);

	// 残像の設定
	m_player->AddAfterImage(true, 5, 1, Math::Color(0.0f, 1.0f, 1.0f, 0.5f), 0.7f);

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	KdAudioManager::Instance().Play("Asset/Sound/Player/JustAttack.WAV", false)->SetVolume(1.0f);

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		if (auto bossEnemy = m_bossEnemy.lock(); bossEnemy)
		{
			m_cameraBossTargetOffset = { 0.0f, 1.0f, -5.0f };
			m_overshootDist = 6.0f;
		}
		else
		{
			m_cameraBossTargetOffset = { 0.0f, 1.0f, -3.5f } ;
			m_overshootDist = 4.0f;
		}
	}

	// 攻撃時はtrueにする
	if (auto katana = m_player->GetKatana().lock(); katana)
	{
		katana->SetNowAttackState(true);
	}

}

void PlayerState_JustAvoidAttack::StateUpdate()
{

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_JustAvoidAttack_end>();
		m_player->ChangeState(state);
		return;
	}
	PlayerStateBase::StateUpdate();

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

	float deltaTime = Application::Instance().GetDeltaTime();

	// 先行ダッシュ処理
	{
		// 敵の奥に行くようにする距離

		// 攻撃方向が指定されていない場合は敵の方向に向かう
		Math::Vector3 toEnemyDir = m_nearestEnemyPos - m_player->GetPos();

		const Math::Vector3 dashDir = (m_attackDirection != Math::Vector3::Zero) ? m_attackDirection : toEnemyDir;

		const Math::Vector3 desiredPoint = m_nearestEnemyPos + dashDir * m_overshootDist;

		Math::Vector3 toDesired = desiredPoint - m_player->GetPos();
		toDesired.y = 0.0f;
		float distance = toDesired.Length();

		const float arriveEps = 1.0f;

		if (distance <= arriveEps)
		{
			m_player->SetIsMoving(Math::Vector3::Zero);

			// キャラを敵の方を向ける
			if (toEnemyDir != Math::Vector3::Zero)
			{
				toEnemyDir.y = 0.0f;
				toEnemyDir.Normalize();
				m_player->UpdateQuaternionDirect(toEnemyDir);
			}

			// カメラをキャラの後ろに回す（セッター使用）
			if (auto camera = m_player->GetPlayerCamera().lock(); camera)
			{
				camera->SetTargetLookAt(m_cameraBossTargetOffset);

				// キャラ前方からヨー角(deg)を計算してカメラ回転に反映
				if (toEnemyDir != Math::Vector3::Zero)
				{
					const float yawRad = std::atan2(toEnemyDir.x, toEnemyDir.z);
					const float yawDeg = DirectX::XMConvertToDegrees(yawRad);
					camera->SetTargetRotation({ 0.0f, yawDeg, 0.0f });
				}
			}
		}
		else
		{

			if (auto effect = m_justAvoidAttackEffect.lock(); effect)
			{
				effect->SetPlayEffect(true);
			}

			toDesired.Normalize();

			// 移動速度の上限
			float maxStep = 10.0f * deltaTime;

			// 今フレームの移動速度
			float speedThisFrame = (distance < maxStep) ? (distance / deltaTime) : 10.0f;

			// 移動ベクトルをセット
			m_player->SetIsMoving(toDesired * speedThisFrame);
			m_time += deltaTime;
		}

	}
}

void PlayerState_JustAvoidAttack::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto effect = m_justAvoidAttackEffect.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	m_player->AddAfterImage();
}