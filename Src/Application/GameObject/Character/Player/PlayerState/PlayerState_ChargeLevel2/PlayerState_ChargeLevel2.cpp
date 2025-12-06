#include "PlayerState_ChargeLevel2.h"
#include"../../../../../main.h"
#include"../PlayerState_Attack1/PlayerState_Attack1.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../PlayerState_Run/PlayerState_Run.h"
#include"../../../../Weapon/Katana/Katana.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../Scene/SceneManager.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Utility/EffectReference.h"

void PlayerState_ChargeLevel2::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeAttack1");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();

	// 複数エフェクト再生
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock()) effect->SetPlayEffect(true);
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

	// スローモーション処理（固定値は既存維持）
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

	// 複数エフェクト停止
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock()) effect->SetPlayEffect(false);
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
	// 複数エフェクトの編集UI
	ImGui::Text("Effects");
	for (size_t i = 0; i < m_playerEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_playerEffects[i]->ImGuiInspector("PlayerState_ChargeLevel2_Effect");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_playerEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_playerEffects.empty()) m_playerEffects.pop_back(); }

	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed);
}

void PlayerState_ChargeLevel2::LoadParametersJson(const nlohmann::json& js)
{
	// エフェクト
	if (js.contains("PlayerState_ChargeLevel2_Effects") && js["PlayerState_ChargeLevel2_Effects"].is_array())
	{
		m_playerEffects.clear();
		for (const auto& node : js["PlayerState_ChargeLevel2_Effects"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_playerEffects.emplace_back(std::move(ref));
		}
	}

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
	// エフェクト
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_playerEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["PlayerState_ChargeLevel2_Effects"] = std::move(arr);

	auto& stateNode = js["PlayerState_ChargeLevel2"];
	auto& playerNode = stateNode["Player"];
	playerNode["blendTime"] = m_stateParameter.blendTime;
	playerNode["animationSpeed"] = m_stateParameter.animationSpeed;
}
