#include "PlayerState_AvoidAttack.h"
#include"../../../../../main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/SpeedAttackEffect/SpeedAttackEffect.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application/GameObject/Character/AfterImage/AfterImage.h"

#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"

void PlayerState_AvoidAttack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("AvoidAttack");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	SceneManager::Instance().GetObjectWeakPtr(m_effect);

	m_player->GetAfterImage()->AddAfterImage(true, 5, 3.0f, Math::Color(0.0f, 1.0f, 1.0f, 1.0f));
}

void PlayerState_AvoidAttack::StateUpdate()
{
	// アニメーション時間
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}
	
	m_player->UpdateAttackCollision(
		m_stateParameter.attackRadius,
		m_stateParameter.attackDistance,
		m_stateParameter.attackCount,
		m_stateParameter.attackInterval,
		m_stateParameter.cameraShake,
		m_stateParameter.cameraTime,
		m_stateParameter.attackStartTime,
		m_stateParameter.attackEndTime
	);

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// Eスキル入力処理
	if (UpdateESkillInput()) return;

	// アニメーション終了後の遷移処理
	if (UpdateSheathKatanaInput()) return;


	// 回避中の移動方向で回転を更新

	Math::Vector3 moveDir = m_player->GetMovement();

	if (moveDir != Math::Vector3::Zero)
	{
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	float deltaTime = Application::Instance().GetDeltaTime();

	Math::Vector3 targetDir = m_nearestEnemyPos - m_player->GetPos();

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraTargetOffset);

		float yaw = std::atan2f(targetDir.x, targetDir.z);
		float yawDeg = DirectX::XMConvertToDegrees(yaw);
		camera->SetTargetRotation({ 0.0f, yawDeg, 0.0f });
	}

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		// 移動を止める
		m_player->SetIsMoving(Math::Vector3::Zero);

		if (auto effect = m_effect.lock())
		{
			effect->SetPlayEffect(true);
		}
	}
	PlayerStateBase::StateUpdate();
}

void PlayerState_AvoidAttack::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto effect = m_effect.lock())
	{
		effect->SetPlayEffect(true);
		effect->StopEffect();
	}

	m_player->GetAfterImage()->AddAfterImage();
}
