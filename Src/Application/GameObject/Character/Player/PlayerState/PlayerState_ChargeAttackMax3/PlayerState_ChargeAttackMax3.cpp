#include "PlayerState_ChargeAttackMax3.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/ChargeAttackEffect_end/ChargeAttackEffect_end.h"

void PlayerState_ChargeAttackMax3::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeMax");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();
	// アニメーション速度を変更
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	SceneManager::Instance().GetObjectWeakPtr(m_effect);

	if (auto effect = m_effect.lock())
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

	// 攻撃中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	m_player->SetIsMoving(Math::Vector3::Zero);

	// 最後に Base 側の StateUpdate を呼び出すことで、フォーカス/方向の追従が反映されます。
	PlayerStateBase::StateUpdate();
}

void PlayerState_ChargeAttackMax3::StateEnd()
{
	PlayerStateBase::StateEnd();
	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraTargetOffset);
		camera->SetDistanceSmooth(m_cameraDistanceSmooth);
		camera->SetRotationSmooth(m_cameraRotationSmooth);
	}

	// 無敵状態解除
	m_player->SetInvincible(false);

	if (auto effect = m_effect.lock())
	{
		effect->SetPlayEffect(false);
		effect->StopEffect();
	}

	// 索敵範囲もとに戻す(Maxの方のImGUiで変更されているデフォルト５だが100になってる。)
	m_searchEnemyRadius = DefaultSearchEnemyRadius;

	// ガードブレイク状態解除
	m_player->SetGuardBreak(false);
}
