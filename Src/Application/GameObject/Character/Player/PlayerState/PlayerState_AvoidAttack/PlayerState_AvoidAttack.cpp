#include "PlayerState_AvoidAttack.h"
#include"../../../../../main.h"
#include"Application/GameObject/Character/Player/PlayerState/PlayerState_SheathKatana/PlayerState_SheathKatana.h"
#include"../../../../../Scene/SceneManager.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Utility/EffectReference.h"
#include"../PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include"Application/GameObject/Character/AfterImage/AfterImage.h"

#include"Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"

#include "Application/GameObject/Utility/EffectReference.h"

void PlayerState_AvoidAttack::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("AvoidAttack");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// 残像
	m_player->GetAfterImage()->AddAfterImage(true, 5, 3.0f, Math::Color(0.0f, 1.0f, 1.0f, 1.0f));
}

void PlayerState_AvoidAttack::StateUpdate()
{
	// アニメーション時間
	{
		m_animeTime = m_player->GetAnimator()->GetPlayProgress();
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

	// 必殺技入力処理
	if (UpdateSpecialAttackInput()) return;

	// Eスキル入力処理
	if (UpdateESkillInput()) return;

	// アニメーション終了後の遷移処理
	if (UpdateSheathKatanaInput()) return;

	float deltaTime = Application::Instance().GetDeltaTime();

	if (m_time < m_stateParameter.dashSpeedTime)
	{
		m_player->SetIsMoving(m_attackDirection * m_stateParameter.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		// 移動を止める
		m_player->SetIsMoving(Math::Vector3::Zero);

		// エフェクト再生
		for (const auto& ref : m_playerEffects)
		{
			if (auto effect = ref->GetEffectBase().lock())
			{
				effect->PlayForTarget<Player>(std::static_pointer_cast<Player>(m_player->GetMyAdls()));
			}
		}
	}
	PlayerStateBase::StateUpdate();
}

void PlayerState_AvoidAttack::StateEnd()
{
	PlayerStateBase::StateEnd();

	// 複数エフェクト停止＋StopEffect
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->StopEffect();
		}
	}

	m_player->GetAfterImage()->AddAfterImage();
}

void PlayerState_AvoidAttack::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_AvoidAttack));
	const auto& p = static_cast<const PlayerState_AvoidAttack&>(other);
	m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	m_playerEffects = p.m_playerEffects;
}

void PlayerState_AvoidAttack::ExposeParametersImGui()
{
	// 複数エフェクトの編集UI
	ImGui::Text("Effects");
	for (size_t i = 0; i < m_playerEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_playerEffects[i]->ImGuiInspector("PlayerState_AvoidAttack_Effect");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_playerEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_playerEffects.empty()) m_playerEffects.pop_back(); }

	m_stateParameter.ExposeImGui();
}

void PlayerState_AvoidAttack::LoadParametersJson(const nlohmann::json& js)
{
	// エフェクト
	if (js.contains("PlayerState_AvoidAttack_Effects") && js["PlayerState_AvoidAttack_Effects"].is_array())
	{
		m_playerEffects.clear();
		for (const auto& node : js["PlayerState_AvoidAttack_Effects"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_playerEffects.emplace_back(std::move(ref));
		}
	}

	if (!js.contains("PlayerState_AvoidAttack")) return;
	const auto& stateNode = js["PlayerState_AvoidAttack"];
	if (stateNode.contains("Player"))
	{
		m_stateParameter.LoadJson(stateNode["Player"]);
	}
}

void PlayerState_AvoidAttack::SaveParametersJson(nlohmann::json& js) const
{
	// エフェクト
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_playerEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["PlayerState_AvoidAttack_Effects"] = std::move(arr);

	m_stateParameter.SaveJson(js["PlayerState_AvoidAttack"]);
}
