#include "PlayerState_ChargeLevel2.h"
#include"../../../../../main.h"
#include"../PlayerState_Attack1/PlayerState_Attack1.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Effect/EffekseerEffect/EffectPlay/EffectPlay.h"
#include"../../../../Effect/EffekseerEffect/SmokeEffect/SmokeEffect.h"
#include"../../../../Effect/EffekseerEffect/ShineEffect/ShineEffect.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

void PlayerState_ChargeLevel2::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack1");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();

	SceneManager::Instance().GetObjectWeakPtr(m_effect);
	SceneManager::Instance().GetObjectWeakPtr(m_smokeEffect);
	SceneManager::Instance().GetObjectWeakPtr(m_shineEffect);

	auto smokeEffect = m_smokeEffect.lock();
	auto shineEffect = m_shineEffect.lock();

	if (auto effect = m_effect.lock(); effect && smokeEffect && shineEffect)
	{
		// スラッシュエフェクトの再生
		effect->SetPlayEffect(true);
		// スモークエフェクトの再生
		smokeEffect->SetPlayEffect(true);
		// キラキラエフェクトの再生
		shineEffect->SetPlayEffect(true);
	}

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt({ 0.f,0.5f,-1.8f });
	}

	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 当たり判定リセット
	m_player->ResetAttackCollision();
}

void PlayerState_ChargeLevel2::StateUpdate()
{
	m_animeTime = m_player->GetAnimator()->GetPlayProgress();

	// スローモーション処理
	if (m_animeTime >= 0.5f && m_animeTime <= 0.6f)
	{
		m_time += Application::Instance().GetUnscaledDeltaTime();

		if (m_time >= 0.0f && m_time <= 0.1f)
		{
			Application::Instance().SetFpsScale(0.1f);
		}
		else
		{
			Application::Instance().SetFpsScale(1.0f);
		}
	}

	// 当たり判定
	m_player->UpdateAttackCollision(6.0f, 2.0f, 5, 0.5f, { 0.4f,0.4f }, 0.3f);

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(60.0f);

	// アニメーションが終了したら納刀する状態へ
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_SheathKatana>();
		m_player->ChangeState(state);
		return;
	}

	PlayerStateBase::StateUpdate();

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

	// 敵の方向を向く
	m_player->SetIsMoving(m_attackDirection);

	// 移動を止める
	m_player->SetIsMoving(Math::Vector3::Zero);
}

void PlayerState_ChargeLevel2::StateEnd()
{
	PlayerStateBase::StateEnd();

	auto smokeEffect = m_smokeEffect.lock();
	auto shineEffect = m_shineEffect.lock();

	if (auto effect = m_effect.lock(); effect && smokeEffect && shineEffect)
	{
		// スラッシュエフェクトの再生
		effect->SetPlayEffect(false);
		// スモークエフェクトの再生
		smokeEffect->SetPlayEffect(false);
		// キラキラエフェクトの再生
		shineEffect->SetPlayEffect(false);
	}

	// 無敵状態解除
	m_player->SetInvincible(false);

}

void PlayerState_ChargeLevel2::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_ChargeLevel2));
	const auto& p = static_cast<const PlayerState_ChargeLevel2&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
}

void PlayerState_ChargeLevel2::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
}

void PlayerState_ChargeLevel2::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_ChargeLevel2")) return;
	const auto& stateNode = js["PlayerState_ChargeLevel2"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
	}
}

void PlayerState_ChargeLevel2::SaveParametersJson(nlohmann::json& js) const
{
	auto& stateNode = js["PlayerState_ChargeLevel2"];
	auto& playerNode = stateNode["Player"];
	playerNode["blendTime"] = m_stateParameter.blendTime;
	playerNode["animationSpeed"] = m_stateParameter.animationSpeed;
}
