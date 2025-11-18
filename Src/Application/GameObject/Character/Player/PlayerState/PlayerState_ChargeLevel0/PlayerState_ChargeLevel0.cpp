#include "PlayerState_ChargeLevel0.h"

#include"Application/main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeAttackMax/PlayerState_ChargeAttackMax.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevel1/PlayerState_ChargeLevel1.h"
#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"

void PlayerState_ChargeLevel0::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack0");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);
	PlayerStateBase::StateStart();

	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		camera->SetTargetLookAt({ 0.f,1.f,-1.5f });
	}

	// 無敵状態にする
	m_player->SetInvincible(true);

	m_time = 0.0f;

	KdAudioManager::Instance().Play("Asset/Sound/Player/Charge.WAV", false)->SetVolume(1.0f);

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(60.0f);

}

void PlayerState_ChargeLevel0::StateUpdate()
{
	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}
	

	if (m_animeTime >= 0.5f)
	{
		auto state = std::make_shared<PlayerState_ChargeLevel1>();
		m_player->ChangeState(state);
		return;
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

}

void PlayerState_ChargeLevel0::StateEnd()
{
	PlayerStateBase::StateEnd();

}
