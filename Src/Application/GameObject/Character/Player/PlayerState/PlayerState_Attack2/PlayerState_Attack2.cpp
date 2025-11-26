#include "PlayerState_Attack2.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../main.h"
#include"../../../../Weapon/Katana/Katana.h"

#include"../PlayerState_Attack3/PlayerState_Attack3.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/Rotation/Rotation.h"
#include"../../../../Effect/EffekseerEffect/AttacEffect1/AttacEffect1.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

void PlayerState_Attack2::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Attack2");
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

	m_lButtonKeyInput = false;				// 次段コンボ予約フラグ初期化

	SceneManager::Instance().GetObjectWeakPtr(m_slashEffect);

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack2.WAV", false)->SetVolume(0.5f);

}

void PlayerState_Attack2::StateUpdate()
{
	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}

	float deltaTime = Application::Instance().GetDeltaTime();

	if (!m_nearestEnemy)
	{
		// カメラ方向に向かって攻撃するようにする。
		m_attackDirection = m_player->GetLastMoveDirection();
		if (m_attackDirection != Math::Vector3::Zero)
		{
			m_player->UpdateQuaternion(m_attackDirection);
		}
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

	m_player->UpdateMoveDirectionFromInput();

	// 回避入力処理
	if (UpdateMoveAvoidInput()) return;

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// Eスキル入力処理
	if (UpdateESkillInput()) return;

	// 先行入力の予約
	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON))
	{
		m_lButtonKeyInput = true;
	}

	UpdateKatanaPos();


	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		if (auto effect = m_slashEffect.lock())
		{
			effect->SetPlayEffect(true);
		}

		// 移動を止める
		m_player->SetIsMoving(Math::Vector3::Zero);

		// 攻撃入力受付
		if (m_animeTime >= m_stateParameter.changeStateTime)
		{
			// 攻撃入力処理
			if (UpdateAttackInput<PlayerState_Attack3>()) return;

			// アニメーション終了後の遷移処理
			if (UpdateSheathKatanaInput()) return;
		}
	}
}

void PlayerState_Attack2::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto effect = m_slashEffect.lock())
	{
		effect->SetPlayEffect(false);
	}
}
