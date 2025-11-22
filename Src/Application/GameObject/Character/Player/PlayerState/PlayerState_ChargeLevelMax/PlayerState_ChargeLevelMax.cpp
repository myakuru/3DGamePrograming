#include "PlayerState_ChargeLevelMax.h"
#include"../../../../../main.h"

#include"../PlayerState_ChargeLevel2/PlayerState_ChargeLevel2.h"
#include"../PlayerState_ChargeAttackMax/PlayerState_ChargeAttackMax.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../../../../Effect/EffekseerEffect/ShineEffectBlue/ShineEffectBlue.h"
#include"../../../../Effect/EffekseerEffect/GroundFreezes/GroundFreezes.h"

void PlayerState_ChargeLevelMax::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack0");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);

	PlayerStateBase::StateStart();

	m_chargeTime = 0.0f;
	m_isCharging = false;

	m_time = 0.0f;

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt({ 0.f,0.5f,-1.7f });
	}

	SceneManager::Instance().GetObjectWeakPtr(m_shineEffect);
	SceneManager::Instance().GetObjectWeakPtr(m_groundFreezes);

	if (auto effect = m_shineEffect.lock(); effect)
	{
		effect->SetPlayEffect(true);
	}

	if (auto effect = m_groundFreezes.lock(); effect)
	{
		effect->SetPlayEffect(true);
	}

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(60.0f);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Charge.WAV", false)->SetVolume(1.0f);
}

void PlayerState_ChargeLevelMax::StateUpdate()
{
	m_animeTime = m_player->GetAnimator()->GetPlayProgress();

	// スローモーション処理
	if (m_animeTime >= 0.5f && m_animeTime <= 0.6f)
	{
		m_time += Application::Instance().GetUnscaledDeltaTime();

		if (m_time >= 0.0f && m_time <= 0.1f)
		{
			Application::Instance().SetFpsScale(0.1f);
		}
		else
		{
			Application::Instance().SetFpsScale(1.0f);
		}
	}

	// 刀は鞘の中にある状態
	UpdateUnsheathed();

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	m_player->SetIsMoving(m_attackDirection);

	// 移動を止める
	m_player->SetIsMoving(Math::Vector3::Zero);

	// アニメーションが終了したらレベル2チャージへ
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_ChargeAttackMax>();
		m_player->ChangeState(state);
		return;
	}
}

void PlayerState_ChargeLevelMax::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		camera->SetTargetLookAt({ 0.f,1.f,-2.5f });
	}

	if (auto effect = m_shineEffect.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}

	if (auto effect = m_groundFreezes.lock(); effect)
	{
		effect->SetPlayEffect(false);
	}
}
