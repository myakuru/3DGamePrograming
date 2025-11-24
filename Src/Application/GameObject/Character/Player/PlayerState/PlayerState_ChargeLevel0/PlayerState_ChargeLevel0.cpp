#include "PlayerState_ChargeLevel0.h"

#include"Application/main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeAttackMax/PlayerState_ChargeAttackMax.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevel1/PlayerState_ChargeLevel1.h"
#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"

/// <summary>
/// ChargeLevel0ステートからMaxまでいき、
/// PlayerState_ChargeAttackMaxからMax3まで遷移する。
/// </summary>

void PlayerState_ChargeLevel0::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack0");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt({ 0.f,1.f,-2.0f });
	}

	// 無敵状態にする
	m_player->SetInvincible(true);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Charge.WAV", false)->SetVolume(1.0f);

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	// 無敵状態にする(PlayerState_ChargeAttackMax3)まで継続
	m_player->SetInvincible(true);

}

void PlayerState_ChargeLevel0::StateUpdate()
{
	// アニメーション時間のデバッグ表示
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
	else
	{
		Application::Instance().SetFpsScale(1.0f);
	}

	// 0.5f
	if (m_animeTime >= m_stateParameter.changeStateTime)
	{
		auto state = std::make_shared<PlayerState_ChargeLevel1>();
		m_player->ChangeState(state);
		return;
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

}

void PlayerState_ChargeLevel0::StateEnd()
{
	PlayerStateBase::StateEnd();

}

void PlayerState_ChargeLevel0::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_ChargeLevel0));
	const auto& p = static_cast<const PlayerState_ChargeLevel0&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_stateParameter.changeStateTime = p.m_stateParameter.changeStateTime;
}

void PlayerState_ChargeLevel0::ExposeParametersImGui()
{
	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::Separator();
	ImGui::DragFloat(U8("状態遷移時間"), &m_stateParameter.changeStateTime);
}

void PlayerState_ChargeLevel0::LoadParametersJson(const nlohmann::json& js)
{
	if (!js.contains("PlayerState_ChargeLevel0")) return;
	const auto& stateNode = js["PlayerState_ChargeLevel0"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("changeStateTime")) m_stateParameter.changeStateTime = playerNode["changeStateTime"].get<float>();
	}
}

void PlayerState_ChargeLevel0::SaveParametersJson(nlohmann::json& js) const
{
	auto& stateNode = js["PlayerState_ChargeLevel0"];
	auto& playerNode = stateNode["Player"];
	playerNode["blendTime"] = m_stateParameter.blendTime;
	playerNode["animationSpeed"] = m_stateParameter.animationSpeed;
	playerNode["changeStateTime"] = m_stateParameter.changeStateTime;
}
