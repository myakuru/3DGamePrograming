#include "PlayerState_ChargeLevelMax.h"
#include"../../../../../main.h"

#include"../PlayerState_ChargeLevel2/PlayerState_ChargeLevel2.h"
#include"../PlayerState_ChargeAttackMax/PlayerState_ChargeAttackMax.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"

#include"../../../../Effect/EffekseerEffect/ShineEffectBlue/ShineEffectBlue.h"
#include"../../../../Effect/EffekseerEffect/GroundFreezes/GroundFreezes.h"

void PlayerState_ChargeLevelMax::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack0");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt({ 0.f,0.5f,-1.7f });
	}

	SceneManager::Instance().GetObjectWeakPtr(m_shineEffect);
	SceneManager::Instance().GetObjectWeakPtr(m_groundFreezes);

	if (auto effect = m_shineEffect.lock())
	{
		effect->SetPlayEffect(true);
	}

	if (auto effect = m_groundFreezes.lock())
	{
		effect->SetPlayEffect(true);
	}

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Charge.WAV", false)->SetVolume(1.0f);
}

void PlayerState_ChargeLevelMax::StateUpdate()
{
	m_animeTime = m_player->GetAnimator()->GetPlayProgress();

	// スローモーション処理
	if (m_animeTime >= m_startSlowMotionTime && m_animeTime <= m_endSlowMotionTime)
	{
		Application::Instance().SetFpsScale(0.1f);
	}
	else
	{
		Application::Instance().SetFpsScale(1.0f);
	}

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

	m_player->SetIsMoving(m_attackDirection);

	// 移動を止める
	m_player->SetIsMoving(Math::Vector3::Zero);

	// アニメーションが終了したらレベル2チャージへ
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_ChargeAttackMax>();
		m_player->ChangeState(state);
		return;
	}
}

void PlayerState_ChargeLevelMax::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraTargetOffset);
	}

	if (auto effect = m_shineEffect.lock())
	{
		effect->SetPlayEffect(false);
	}

	if (auto effect = m_groundFreezes.lock())
	{
		effect->SetPlayEffect(false);
	}
}

void PlayerState_ChargeLevelMax::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_ChargeLevelMax));
	const auto& p = static_cast<const PlayerState_ChargeLevelMax&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_startSlowMotionTime = p.m_startSlowMotionTime;
	m_endSlowMotionTime = p.m_endSlowMotionTime;
}

void PlayerState_ChargeLevelMax::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::DragFloat(U8("スローモーション開始時間"), &m_startSlowMotionTime, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat(U8("スローモーション終了時間"), &m_endSlowMotionTime, 0.01f, 0.0f, 1.0f);
}

void PlayerState_ChargeLevelMax::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_ChargeLevelMax")) return;
	const auto& stateNode = js["PlayerState_ChargeLevelMax"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("startSlowMotionTime")) m_startSlowMotionTime = playerNode["startSlowMotionTime"].get<float>();
		if (playerNode.contains("endSlowMotionTime")) m_endSlowMotionTime = playerNode["endSlowMotionTime"].get<float>();
	}
}

void PlayerState_ChargeLevelMax::SaveParametersJson(nlohmann::json& js) const
{
	auto& stateNode = js["PlayerState_ChargeLevelMax"];
	auto& playerNode = stateNode["Player"];
	playerNode["blendTime"] = m_stateParameter.blendTime;
	playerNode["animationSpeed"] = m_stateParameter.animationSpeed;
	playerNode["startSlowMotionTime"] = m_startSlowMotionTime;
	playerNode["endSlowMotionTime"] = m_endSlowMotionTime;
}
