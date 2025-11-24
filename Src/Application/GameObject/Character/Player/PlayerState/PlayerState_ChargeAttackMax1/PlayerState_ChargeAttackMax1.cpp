#include "PlayerState_ChargeAttackMax1.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_ChargeAttackMax2/PlayerState_ChargeAttackMax2.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/ChargeAttackEffect/ChargeAttackEffect.h"

void PlayerState_ChargeAttackMax1::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeMax");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();
	// アニメーション速度を変更
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	SceneManager::Instance().GetObjectWeakPtr(m_effect);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack4.WAV", false)->SetVolume(1.0f);

	if(auto effect = m_effect.lock())
	{
		// エフェクトの初期化
		effect->SetPlayEffect(true);
	}

	// 当たり判定リセット
	m_player->ResetAttackCollision();

}

void PlayerState_ChargeAttackMax1::StateUpdate()
{
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_ChargeAttackMax2>();
		m_player->ChangeState(state);
		return;
	}

	// 攻撃の当たり判定更新
	m_player->UpdateAttackCollision(
		m_stateParameter.attackRadius,
		m_stateParameter.attackDistance,
		m_stateParameter.attackCount,
		m_stateParameter.attackInterval,
		m_stateParameter.cameraShake,
		m_stateParameter.cameraTime
	);

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

void PlayerState_ChargeAttackMax1::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto effect = m_effect.lock())
	{
		// エフェクトの初期化
		effect->SetPlayEffect(false);
		effect->StopEffect();
	}
}
