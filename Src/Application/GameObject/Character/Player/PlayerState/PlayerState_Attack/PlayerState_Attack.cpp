#include "PlayerState_Attack.h"
#include"../../../../../main.h"
#include"../PlayerState_Attack1/PlayerState_Attack1.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../Weapon/WeaponKatanaScabbard/WeaponKatanaScabbard.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/SwordFlash/SwordFlash.h"

#include"../PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include"../PlayerState_FowardAvoid/PlayerState_FowardAvoid.h"

#include"../PlayerState_Skill/PlayerState_Skill.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"

#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"


void PlayerState_Attack::StateStart()
{
	PlayerStateBase::StateStart();

	// アニメーション再生
	auto anime = m_player->GetAnimeModel()->GetAnimation("Attack");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

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

	// エフェクト取得
	SceneManager::Instance().GetObjectWeakPtr(m_slashEffect);
	// アニメーション速度設定
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack.wav", false)->SetVolume(1.0f);
}

void PlayerState_Attack::StateUpdate()
{
	// アニメーション時間の取得
	m_animeTime = m_player->GetAnimator()->GetPlayProgress();

	float deltaTime = Application::Instance().GetDeltaTime();

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

	Math::Vector3 toEnemyDir = m_nearestEnemyPos - m_player->GetPos();

	// キャラを敵の方向へ
	if (toEnemyDir != Math::Vector3::Zero)
	{
		toEnemyDir.y = 0.0f;
		toEnemyDir.Normalize();
		m_player->UpdateQuaternionDirect(toEnemyDir);
	}

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraTargetOffset);

		if (toEnemyDir != Math::Vector3::Zero)
		{
			const float yaw = std::atan2f(toEnemyDir.x, toEnemyDir.z);
			const float yawDeg = DirectX::XMConvertToDegrees(yaw);
			camera->SetTargetRotation({ 0.0f, yawDeg, 0.0f });
		}

	}

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

	// 先行ダッシュ処理
	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		// エフェクト再生・移動停止
		if (auto effect = m_slashEffect.lock())
		{
			effect->SetPlayEffect(true);
		}

		m_player->SetIsMoving(m_stateParameter.moveSpeed);

		if (m_animeTime >= m_stateParameter.changeStateTime)
		{

			// 攻撃入力処理
			if (UpdateAttackInput<PlayerState_Attack1>()) return;

			// アニメーション終了後の遷移処理
			if (UpdateSheathKatanaInput()) return;
		}
	}

	PlayerStateBase::StateUpdate();

	UpdateKatanaPos();
}

void PlayerState_Attack::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto swordEffect = m_slashEffect.lock())
	{
		swordEffect->SetPlayEffect(false);
	}
}

void PlayerState_Attack::ExposeParametersImGui()
{
	m_stateParameter.ExposeImGui();
}

void PlayerState_Attack::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_Attack")) return;
	const auto& stateNode = js["PlayerState_Attack"];
	if (stateNode.contains("Player")) m_stateParameter.LoadJson(stateNode["Player"]);
}

void PlayerState_Attack::SaveParametersJson(nlohmann::json& js) const
{
	m_stateParameter.SaveJson(js["PlayerState_Attack"]);
	js["PlayerState_Attack"]["m_dashSpeed"] = m_stateParameter.dashSpeed;
}
