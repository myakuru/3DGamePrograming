#include "PlayerState_ChargeAttackMax3.h"
#include"../PlayerState_Idle/PlayerState_Idle.h"
#include"../../../../Camera/PlayerCamera/PlayerCamera.h"
#include"../../../../../Scene/SceneManager.h"

#include "Application/GameObject/Utility/EffectReference.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"

void PlayerState_ChargeAttackMax3::StateStart()
{
	auto anime = m_player->GetAnimeModel()->GetAnimation("ChargeMax");
	m_player->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	PlayerStateBase::StateStart();
	// アニメーション速度を変更
	m_player->SetAnimeSpeed(m_stateParameter.animationSpeed);

	KdAudioManager::Instance().Play("Asset/Sound/Player/Attack4.WAV", false)->SetVolume(1.0f);

	// 当たり判定リセット
	m_player->ResetAttackCollision();

	// エフェクト再生・移動停止（複数）
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->PlayForTarget<Player>(std::static_pointer_cast<Player>(m_player->GetMyAdls()));
		}
	}

}

void PlayerState_ChargeAttackMax3::StateUpdate()
{
	if (m_player->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<PlayerState_Idle>();
		m_player->ChangeState(state);
		return;
	}

	// 攻撃の当たり判定更新
	m_player->UpdateAttackCollision(
		m_stateParameter.attackRadius,
		m_stateParameter.attackDistance,
		m_stateParameter.attackCount,
		m_stateParameter.attackInterval,
		m_stateParameter.cameraShake,
		m_stateParameter.cameraTime
	);

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

	m_player->SetIsMoving(Math::Vector3::Zero);

	// 最後に Base 側の StateUpdate を呼び出すことで、フォーカス/方向の追従が反映されます。
	PlayerStateBase::StateUpdate();
}

void PlayerState_ChargeAttackMax3::StateEnd()
{
	PlayerStateBase::StateEnd();
	if (auto camera = m_player->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraTargetOffset);
		camera->SetDistanceSmooth(m_cameraDistanceSmooth);
		camera->SetRotationSmooth(m_cameraRotationSmooth);
	}

	// エフェクト停止（複数）
	for (const auto& ref : m_playerEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->StopEffect();
		}
	}

	// 無敵状態解除
	m_player->SetInvincible(false);

	// 索敵範囲もとに戻す(Maxの方のImGUiで変更されているデフォルト５だが100になってる。)
	m_searchEnemyRadius = DefaultSearchEnemyRadius;

	// ガードブレイク状態解除
	m_player->SetGuardBreak(false);
}

void PlayerState_ChargeAttackMax3::ApplyFromConfig(const PlayerStateBase& other)
{
	assert(typeid(other) == typeid(PlayerState_ChargeAttackMax3));
	const auto& p = static_cast<const PlayerState_ChargeAttackMax3&>(other);
	m_stateParameter = p.m_stateParameter;  // 構造体一括コピー
	m_playerEffects = p.m_playerEffects;
}

void PlayerState_ChargeAttackMax3::ExposeParametersImGui()
{
	// 複数エフェクトの編集UI
	ImGui::Text("Effects");
	for (size_t i = 0; i < m_playerEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_playerEffects[i]->ImGuiInspector("PlayerState_ChargeAttackMax3_Effect");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_playerEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_playerEffects.empty()) m_playerEffects.pop_back(); }

	m_stateParameter.ExposeImGui();
}

void PlayerState_ChargeAttackMax3::LoadParametersJson(const nlohmann::json& js)
{
	// 複数配列のみ
	if (js.contains("PlayerState_ChargeAttackMax3_Effect") && js["PlayerState_ChargeAttackMax3_Effect"].is_array())
	{
		m_playerEffects.clear();
		for (const auto& node : js["PlayerState_ChargeAttackMax3_Effect"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_playerEffects.emplace_back(std::move(ref));
		}
	}

	if (!js.contains("PlayerState_ChargeAttackMax3")) return;
	const auto& stateNode = js["PlayerState_ChargeAttackMax3"];
	if (stateNode.contains("Player"))
	{
		m_stateParameter.LoadJson(stateNode["Player"]);
	}
}

void PlayerState_ChargeAttackMax3::SaveParametersJson(nlohmann::json& js) const
{
	// 複数配列
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_playerEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["PlayerState_ChargeAttackMax3_Effect"] = std::move(arr);

	m_stateParameter.SaveJson(js["PlayerState_ChargeAttackMax3"]);
}
