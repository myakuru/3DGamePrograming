#include "PlayerState_AvoidAttack.h"
#include"../../../../../main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/SpeedAttackEffect/SpeedAttackEffect.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application\GameObject\Character\AfterImage\AfterImage.h"

void PlayerState_AvoidAttack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("AvoidAttack");
	m_player->GetAnimator()->SetAnimation(anime, 0.25f, false);
	PlayerStateBase::StateStart();
	m_time = 0.0f;

	// 当たり判定リセット
	m_LButtonkeyInput = false;

	m_player->ResetAttackCollision();

	SceneManager::Instance().GetObjectWeakPtr(m_effect);

	m_player->GetAfterImage()->AddAfterImage(true, 5, 1.0f, Math::Color(0.0f, 1.0f, 1.0f, 1.0f));
}

void PlayerState_AvoidAttack::StateUpdate()
{
	// アニメーション時間
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();

		m_maxAnimeTime = m_player->GetAnimator()->GetMaxAnimationTime();
	}

	if (m_animeTime >= 0.0f && m_animeTime <= 0.1f)
	{
		m_player->UpdateAttackCollision(5.0f, 5.0f, 1, m_maxAnimeTime, { 0.3f, 0.0f }, 0.3f);
	}

	if (KeyboardManager::GetInstance().IsKeyJustPressed('Q'))
	{
		if (m_playerData.GetPlayerStatus().specialPoint == m_playerData.GetPlayerStatus().specialPointMax)
		{
			m_playerData.SetPlayerStatus().specialPoint = 0;
			auto specialAttackState = std::make_shared<PlayerState_SpecialAttackCutIn>();
			m_player->ChangeState(specialAttackState);
			return;
		}
	}

	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto runState = std::make_shared<PlayerState_SheathKatana>();
		m_player->ChangeState(runState);
		return;
	}
	// 回避中の移動方向で回転を更新
	if (m_player->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = m_player->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		m_player->UpdateQuaternionDirect(moveDir);
	}

	float deltaTime = Application::Instance().GetDeltaTime();

	m_time += deltaTime;

	if (m_time < 0.3f)
	{
		float dashSpeed = 2.0f;
		m_player->SetIsMoving(m_attackDirection * dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		// 移動を止める
		m_player->SetIsMoving(Math::Vector3::Zero);

		if (auto effect = m_effect.lock(); effect)
		{
			effect->SetPlayEffect(true);
		}
	}
	PlayerStateBase::StateUpdate();
}

void PlayerState_AvoidAttack::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto effect = m_effect.lock(); effect)
	{
		effect->SetPlayEffect(true);
		effect->StopEffect();
	}

	m_player->GetAfterImage()->AddAfterImage();
}
