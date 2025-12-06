#include"PlayerState_ChargeLevel1.h"
#include"Application/main.h"
#include"Application/GameObject/Weapon/Katana/Katana.h"
#include"Application/Scene/SceneManager.h"
#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"

#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include"Application/GameObject/Utility/EffectReference.h"

#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevel2/PlayerState_ChargeLevel2.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeAttackMax/PlayerState_ChargeAttackMax.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelMax/PlayerState_ChargeLevelMax.h"

void PlayerState_ChargeLevel1::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack0");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);

	PlayerStateBase::StateStart();

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt({ 0.f,0.5f,-1.9f });
	}

	// 複数エフェクト再生
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->SetPlayEffect(true);
		}
	}

	// アニメーション速度を変更
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Charge.WAV", false)->SetVolume(1.0f);

}

void PlayerState_ChargeLevel1::StateUpdate()
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

	// アニメーションが終了したらレベル1チャージへ
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_ChargeLevelMax>();
		m_player->ChangeState(state);
		return;
	}

}

void PlayerState_ChargeLevel1::StateEnd()
{
	PlayerStateBase::StateEnd();

	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraTargetOffset);
	}

	// 複数エフェクト停止
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->SetPlayEffect(false);
		}
	}
}

void PlayerState_ChargeLevel1::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_ChargeLevel1));
	const auto& p = static_cast<const PlayerState_ChargeLevel1&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_startSlowMotionTime = p.m_startSlowMotionTime;
	m_endSlowMotionTime = p.m_endSlowMotionTime;
}

void PlayerState_ChargeLevel1::ExposeParametersImGui()
{
	// 複数エフェクトの編集UI
	ImGui::Text("Effects");
	for (size_t i = 0; i < m_playerEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_playerEffects[i]->ImGuiInspector("PlayerState_ChargeLevel1_Effect");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_playerEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_playerEffects.empty()) m_playerEffects.pop_back(); }

	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
	ImGui::DragFloat(U8("スローモーション開始時間"), &m_startSlowMotionTime);
	ImGui::DragFloat(U8("スローモーション終了時間"), &m_endSlowMotionTime);
}

void PlayerState_ChargeLevel1::LoadParametersJson(const nlohmann::json& js)
{
	// エフェクト
	if (js.contains("PlayerState_ChargeLevel1_Effects") && js["PlayerState_ChargeLevel1_Effects"].is_array())
	{
		m_playerEffects.clear();
		for (const auto& node : js["PlayerState_ChargeLevel1_Effects"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_playerEffects.emplace_back(std::move(ref));
		}
	}

	if (!js.contains("PlayerState_ChargeLevel1")) return;
	const auto& stateNode = js["PlayerState_ChargeLevel1"];
	if (stateNode.contains("Player"))
	{
		const auto& playerNode = stateNode["Player"];
		if (playerNode.contains("blendTime")) m_stateParameter.blendTime = playerNode["blendTime"].get<float>();
		if (playerNode.contains("animationSpeed")) m_stateParameter.animationSpeed = playerNode["animationSpeed"].get<float>();
		if (playerNode.contains("startSlowMotionTime")) m_startSlowMotionTime = playerNode["startSlowMotionTime"].get<float>();
		if (playerNode.contains("endSlowMotionTime")) m_endSlowMotionTime = playerNode["endSlowMotionTime"].get<float>();
	}
}

void PlayerState_ChargeLevel1::SaveParametersJson(nlohmann::json& js) const
{
	// エフェクト
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_playerEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["PlayerState_ChargeLevel1_Effects"] = std::move(arr);

	auto& stateNode = js["PlayerState_ChargeLevel1"];
	auto& playerNode = stateNode["Player"];
	playerNode["blendTime"] = m_stateParameter.blendTime;
	playerNode["animationSpeed"] = m_stateParameter.animationSpeed;
	playerNode["startSlowMotionTime"] = m_startSlowMotionTime;
	playerNode["endSlowMotionTime"] = m_endSlowMotionTime;
}
