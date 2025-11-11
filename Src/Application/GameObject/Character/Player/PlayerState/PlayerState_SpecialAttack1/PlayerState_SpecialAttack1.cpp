#include "PlayerState_SpecialAttack1.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../main.h"
#include"../../../../Effect/EffekseerEffect/SpecialAttack1/SpecialAttack1.h"
#include"../../../../Effect/EffekseerEffect/SpecialAttackSmoke/SpecialAttackSmoke.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"


void PlayerState_SpecialAttack1::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack0");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);
	PlayerStateBase::StateStart();

	SceneManager::Instance().GetObjectWeakPtr(m_effect);
	SceneManager::Instance().GetObjectWeakPtr(m_smokeEffect);

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// アニメーション再生速度を変更
	m_player->SetAnimeSpeed(10.0f);

	m_playerData.SetPlayerStatus().specialPoint = 0;

	m_playSound = false;
}

void PlayerState_SpecialAttack1::StateUpdate()
{

	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();

		m_maxAnimeTime = m_player->GetAnimator()->GetMaxAnimationTime();

		if (m_animeTime > m_maxAnimeTime)
		{
			KdDebugGUI::Instance().AddLog(U8("Attackアニメ時間: %f"), m_animeTime);
			KdDebugGUI::Instance().AddLog("\n");
		}
	}

	// 当たり判定有効時間: 最初の0.5秒のみ

	if (m_animeTime >= 0.4f)
	{
		if (auto effect = m_smokeEffect.lock(); effect)
		{
			effect->SetPlayEffect(true);
		}

		if (auto effect = m_effect.lock())
		{
			effect->SetPlayEffect(true);
		}

		if (!m_playSound)
		{
			KdAudioManager::Instance().Play("Asset/Sound/Player/SpecialAttackEnd.WAV", false)->SetVolume(0.5f);
			m_playSound = true;
		}

		m_player->UpdateAttackCollision(10.0f, 7.0f, 6, 0.3f, { 0.4f, 0.4f }, 0.5f, 0.0f, 1.8f);
	}

	Math::Vector3 moveDir = m_player->GetMovement();

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		// キャラ前方からヨー角(deg)を計算してカメラ回転に反映
		if (moveDir != Math::Vector3::Zero)
		{
			moveDir.Normalize();
			m_yawRad = std::atan2(moveDir.x, moveDir.z);
			m_yawDeg = DirectX::XMConvertToDegrees(m_yawRad);
			camera->SetTargetRotation({ -15.0f, m_yawDeg , 0.0f });
		}
	}
	m_player->SetIsMoving(m_attackDirection);

	UpdateKatanaPos();

	// 移動を止める
	m_player->SetIsMoving(Math::Vector3::Zero);

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_SheathKatana>();
		m_player->ChangeState(state);
		return;
	}
}

void PlayerState_SpecialAttack1::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto effect = m_effect.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	if (auto effect = m_smokeEffect.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	m_player->SetInvincible(false);
}
