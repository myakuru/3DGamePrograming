#include "PlayerState_Attack3.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../main.h"
#include"../PlayerState_Attack4/PlayerState_Attack4.h"

#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/SlashEffect/SlashEffect.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"
#include"../PlayerState_Skill/PlayerState_Skill.h"
	
#include"../../../../Weapon/Katana/Katana.h"
#include"Application/GameObject/Character/EnemyBase/BossEnemy/BossEnemy.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application\GameObject\Character\AfterImage\AfterImage.h"

void PlayerState_Attack3::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("Attack3");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	// 攻撃時はtrueにする
	for (const auto& katanaWeak : m_player->GetKatanas())
	{
		if (auto katana = katanaWeak.lock())
		{
			katana->SetNowAttackState(true);
		}
	}

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	SceneManager::Instance().GetObjectWeakPtr(m_slashEffect);

	if (auto effect = m_slashEffect.lock())
	{
		effect->SetPlayEffect(true);
	}

	SceneManager::Instance().GetObjectWeakPtr(m_bossEnemy);

	// カメラの位置を変更
	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		if (auto bossEnemy = m_bossEnemy.lock())
		{
			camera->SetTargetLookAt({ 0.0,1.0f,-7.5f });
		}
		else
		{
			camera->SetTargetLookAt({ 0.0,1.0f,-4.5f });
		}
	}

	// 残像の設定
	m_player->GetAfterImage()->AddAfterImage(true, 3, 1, Math::Color(0.0f, 1.0f, 1.0f, 1.0f));

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack3.WAV", false)->SetVolume(0.5f);
}

void PlayerState_Attack3::StateUpdate()
{
	UpdateKatanaPos();

	// アニメーション時間のデバッグ表示
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
	}

	PlayerStateBase::StateUpdate();

	float deltaTime = Application::Instance().GetDeltaTime();

	if (m_attackDirection != Math::Vector3::Zero)
	{
		m_player->UpdateQuaternionDirect(m_attackDirection);
	}

	// 0.5秒間当たり判定有効
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

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		// 移動を止める
		m_player->SetIsMoving(Math::Vector3::Zero);

		// 攻撃入力受付
		if (m_animeTime >= m_stateParameter.changeStateTime)
		{
			// 攻撃入力処理
			if (UpdateAttackInput<PlayerState_Attack4>()) return;

			// アニメーション終了後の遷移処理
			if (UpdateSheathKatanaInput()) return;
		}
	}

	// 最後に Base 側の StateUpdate を呼び出すことで、フォーカス/方向の追従が反映されます。
	PlayerStateBase::StateUpdate();

}

void PlayerState_Attack3::StateEnd()
{
	PlayerStateBase::StateEnd();

	// エフェクトの停止
	if (auto effect = m_slashEffect.lock())
	{
		effect->SetPlayEffect(false);
	}

	m_player->GetAfterImage()->AddAfterImage();
}
