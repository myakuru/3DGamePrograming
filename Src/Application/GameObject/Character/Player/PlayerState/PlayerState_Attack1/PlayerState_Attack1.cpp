#include "PlayerState_Attack1.h"
#include"../PlayerState_Attack2/PlayerState_Attack2.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../../main.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../../../../Effect/EffekseerEffect/Rotation/Rotation.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_Attack1::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Attack1");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	

	PlayerStateBase::StateStart();

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// 攻撃時はtrueにする
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetNowAttackState(true);
		}
	}

	m_effectOnce = false;

	m_lButtonKeyInput = false;

	// エフェクトの取得
	SceneManager::Instance().GetObjectWeakPtr(m_effect);

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack1.wav", false)->SetVolume(0.5f);
}

void PlayerState_Attack1::StateUpdate()
{
	UpdateKatanaPos();

	// アニメーション時間の取得
	m_animeTime = m_player->GetAnimator()->GetPlayProgress();

	float deltaTime = Application::Instance().GetDeltaTime();

	if (m_attackDirection != Math::Vector3::Zero)
	{
		m_player->UpdateQuaternionDirect(m_attackDirection);
	}

	// 判定有効
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

	m_player->UpdateMoveDirectionFromInput();

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// 先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		m_lButtonKeyInput = true;
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

		// エフェクトの再生
		if (auto effet = m_effect.lock())
		{
			effet->SetPlayEffect(true);
		}

		if (auto playreCamera = m_player->GetPlayerCamera().lock())
		{
			playreCamera->StartShake({ m_player->GetCameraShakePower()}, m_player->GetCameraShakeTime());
		}

		// 攻撃入力受付
		if (m_animeTime >= m_stateParameter.changeStateTime)
		{
			// Eスキル入力処理
			if (UpdateESkillInput()) return;

			// 攻撃入力処理
			if (UpdateAttackInput<PlayerState_Attack2>()) return;

			// アニメーション終了後の遷移処理
			if (UpdateSheathKatanaInput()) return;
		}

	}

	// 最後に Base 側の StateUpdate を呼び出すことで、フォーカス/方向の追従が反映されます。
	PlayerStateBase::StateUpdate();
}

void PlayerState_Attack1::StateEnd()
{
	PlayerStateBase::StateEnd();

	// エフェクトがあったらフラグをfalseにする
	if(auto effect = m_effect.lock())
	{
		effect->SetPlayEffect(false);
		effect->StopEffect();
	}

	// カタナの軌跡を消す
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetNowAttackState(false);
		}
	}
}
