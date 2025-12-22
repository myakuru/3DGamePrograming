#include "PlayerState_HitBase.h"
#include "Application/main.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_Idle/PlayerState_Idle.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "MyFramework/Manager/JsonManager/JsonManager.h"

void PlayerState_HitBase::StateStart(Player* owner)
{
	// アニメ開始
	auto anime = owner->GetAnimeModel()->GetAnimation(GetAnimationName());
	owner->GetAnimator()->SetAnimation(anime, m_params.blendTime, false);
	owner->SetAnimeSpeed(m_params.animationSpeed);

	// ヒット時：移動入力は無効化、向きは維持
	owner->SetIsMoving(Math::Vector3::Zero);

	// ノックバック適用（ワンショット速度加算等、専用APIがあれば差し替え）
	if (GetKnockback() != Math::Vector3::Zero)
	{
		const Math::Vector3 kb = GetKnockback();
		owner->SetIsMoving(kb);
	}

	// 無敵化（設定次第）
	owner->SetInvincible(IsInvincibleDuringHit());

	// エフェクト開始
	for (const auto& ref : m_effects) { if (auto e = ref->GetEffectBase().lock()) e->PlayForTarget<Player>(std::static_pointer_cast<Player>(owner->GetMyAdls())); }

	m_time = 0.0f;
	OnStartExtra(owner);
}

void PlayerState_HitBase::StateUpdate(Player* owner)
{
	const float dt = Application::Instance().GetDeltaTime();

	// ヒット中はプレイヤー入力による移動を止める
	owner->SetIsMoving(Math::Vector3::Zero);

	// 刀位置などの共通更新（見た目崩れ防止）
	PlayerStateServices::UpdateKatanaPos(owner);

	// カメラ演出が必要ならここで
	// 例：軽い揺れ、ズーム等（必要時に OnUpdateExtra 内で実施）

	// 終了条件
	bool timeOver = (m_time >= GetStunDuration());
	bool animEnd = owner->GetAnimator()->IsAnimationEnd();

	OnUpdateExtra(owner, dt);

	m_time += dt;

	if ((EndOnAnimationFinished() && animEnd) || (!EndOnAnimationFinished() && timeOver))
	{
		PlayerInputServices::TryChange<PlayerState_Idle>(this);
	}
}

void PlayerState_HitBase::StateEnd(Player* owner)
{
	for (const auto& ref : m_effects) { if (auto e = ref->GetEffectBase().lock()) e->StopEffect(); }

	// 無敵解除（入っていた場合）
	if (IsInvincibleDuringHit()) owner->SetInvincible(false);

	OnEndExtra(owner);
}

void PlayerState_HitBase::ExposeParametersImGui()
{
	m_params.ExposeImGui();

	ImGui::Text("Effects");
	for (size_t i = 0; i < m_effects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_effects[i]->ImGuiInspector(GetImGuiEffectLabel());
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_effects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_effects.empty()) m_effects.pop_back(); }

	ImGui::Separator();
	ImGui::DragFloat3(U8("ノックバック"), &m_knockback.x, 0.01f);
}

void PlayerState_HitBase::JsonInput(const nlohmann::json& js)
{
	// StateParameter
	const char* key = GetStateJsonKey();
	if (js.contains(key)) m_params.LoadJson(js[key]); else m_params.LoadJson(js);

	// Effects
	const char* effKey = GetEffectsJsonKey();
	if (js.contains(effKey) && js[effKey].is_array())
	{
		m_effects.clear();
		for (const auto& node : js[effKey])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_effects.emplace_back(std::move(ref));
		}
	}

	// knockback
	if (js.contains(key))
	{
		const auto& node = js[key];
		if (node.contains("Player") && node["Player"].contains("knockback"))
		{
			m_knockback = JSON_MANAGER.JsonToVector(node["Player"]["knockback"]);
		}
	}
}

void PlayerState_HitBase::JsonSave(nlohmann::json& js) const
{
	const char* key = GetStateJsonKey();
	if (!js.contains(key)) js[key] = nlohmann::json::object();
	m_params.SaveJson(js[key]);

	// Effects
	const char* effKey = GetEffectsJsonKey();
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_effects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js[effKey] = std::move(arr);

	// knockback
	js[key]["Player"]["knockback"] = JSON_MANAGER.VectorToJson(m_knockback);
}