#include "PlayerState_ChargeAttackMax3.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/ChargeAttackEffect_end/ChargeAttackEffect_end.h"

void PlayerState_ChargeAttackMax3::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeMax");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);
	PlayerStateBase::StateStart();
	// アニメーション速度を変更
	m_player->SetAnimeSpeed(60.0f);

	SceneManager::Instance().GetObjectWeakPtr(m_effect);

	if (auto effect = m_effect.lock(); effect)
	{
		effect->SetPlayEffect(true);
	}

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack4.WAV", false)->SetVolume(1.0f);

	// 当たり判定リセット
	m_player->ResetAttackCollision();

}

void PlayerState_ChargeAttackMax3::StateUpdate()
{
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(state);
		return;
	}

	// 攻撃の当たり判定更新
	m_player->UpdateAttackCollision(15.0f, 1.0f, 5.0f, m_maxAnimeTime, { 0.3f, 0.3f }, 0.1f);

	// 刀は鞘の中にある状態
	UpdateUnsheathed();

	PlayerStateBase::StateUpdate();

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	m_player->SetIsMoving(Math::Vector3::Zero);
}

void PlayerState_ChargeAttackMax3::StateEnd()
{
	PlayerStateBase::StateEnd();
	if (auto camera = m_player->GetPlayerCamera().lock(); camera)
	{
		camera->SetTargetLookAt(Math::Vector3(0.0f, 1.0f, -3.5f));
		camera->SetDistanceSmooth(8.f);
		camera->SetRotationSmooth(8.f);
	}

	// 無敵状態解除
	m_player->SetInvincible(false);

	if (auto effect = m_effect.lock(); effect)
	{
		effect->SetPlayEffect(false);
		effect->StopEffect();
	}
}
