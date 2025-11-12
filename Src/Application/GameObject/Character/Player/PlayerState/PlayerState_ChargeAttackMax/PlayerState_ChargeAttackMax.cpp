#include "PlayerState_ChargeAttackMax.h"
#include"../../../../../main.h"
#include"../PlayerState_ChargeAttackMax1/PlayerState_ChargeAttackMax1.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"Application\GameObject\Character\AfterImage\AfterImage.h"

void PlayerState_ChargeAttackMax::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Eskill");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);
	PlayerStateBase::StateStart();
	m_time = 0.0f;

	// 敵との当たり判定を無効化
	m_player->SetAtkPlayer(true);

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(100.0f);

	KdAudioManager::Instance().Play("Asset/Sound/Player/ChargeAttack.WAV", false)->SetVolume(1.0f);

	// 残像の設定
	m_player->GetAfterImage()->AddAfterImage(true, 5, 0.1f, Math::Color(0.0f, 1.0f, 1.0f, 1.0f));

	// 当たり判定リセット
	m_player->ResetAttackCollision();
}

void PlayerState_ChargeAttackMax::StateUpdate()
{

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_ChargeAttackMax1>();
		m_player->ChangeState(state);
		return;
	}
	PlayerStateBase::StateUpdate();

	UpdateKatanaPos();

	// 攻撃の当たり判定更新
	m_player->UpdateAttackCollision(15.0f, 1.0f, 2.0f, m_maxAnimeTime, { 0.0f, 0.0f }, 0.1f);

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

			// キャラを敵の方にキャラの体を向ける
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

void PlayerState_ChargeAttackMax::StateEnd()
{
	PlayerStateBase::StateEnd();
	// 敵との当たり判定をもとに戻す
	m_player->SetAtkPlayer(false);

	// 残像の設定を元に戻す
	m_player->GetAfterImage()->AddAfterImage();
}
