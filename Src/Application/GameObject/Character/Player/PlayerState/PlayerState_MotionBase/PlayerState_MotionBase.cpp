#include "PlayerState_MotionBase.h"
#include "Application/main.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Character/Player/PlayerConfig.h"

void PlayerState_MotionBase::StateStart(Player* _owner)
{
	if (_owner && _owner->GetPlayerConfig())
	{
		_owner->GetPlayerConfig()->ApplyPrototypeParametersTo(*this);
	}

	// アニメ開始
	auto anime = _owner->GetAnimeModel()->GetAnimation(GetAnimationName());
	_owner->GetAnimator()->SetAnimation(anime, m_params.blendTime, false);
	_owner->SetAnimeSpeed(m_params.animationSpeed);

	// エフェクト開始
	for (const auto& ref : m_effects) { if (auto e = ref->GetEffectBase().lock()) e->PlayForTarget<Player>(std::static_pointer_cast<Player>(_owner->GetMyAdls())); }

	// SE
	if (const char* se = GetMoveSE()) { KdAudioManager::Instance().Play(se, false)->SetVolume(1.0f); }

	m_time = 0.0f;
	OnStartExtra(_owner);
}

void PlayerState_MotionBase::StateUpdate(Player* _owner)
{
	const float dt = Application::Instance().GetDeltaTime();

	// 入力から移動更新
	_owner->UpdateMoveDirectionFromInput();

	// 向き更新（移動がある場合のみ）
	if (_owner->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 dir = _owner->GetMovement(); dir.y = 0.0f; dir.Normalize();
		_owner->UpdateQuaternionDirect(dir);
	}

	// 実際の移動適用（必要なら速度係数）
	_owner->SetIsMoving(_owner->GetMovement() * m_params.dashSpeed); // locomotion用にdashSpeedを移動係数として再利用

	// 追加処理フック
	OnUpdateExtra(_owner, dt);

	m_time += dt;
}

void PlayerState_MotionBase::StateEnd(Player* _owner)
{
	for (const auto& ref : m_effects) { if (auto e = ref->GetEffectBase().lock()) e->StopEffect(); }
	OnEndExtra(_owner);
}

void PlayerState_MotionBase::ExposeParametersImGui()
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
}

void PlayerState_MotionBase::JsonInput(const nlohmann::json& _js)
{
	// StateParameter
	const char* key = GetStateJsonKey();
	if (_js.contains(key)) m_params.LoadJson(_js[key]); else m_params.LoadJson(_js);

	// Effects
	const char* effKey = GetEffectsJsonKey();
	if (_js.contains(effKey) && _js[effKey].is_array())
	{
		m_effects.clear();
		for (const auto& node : _js[effKey])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_effects.emplace_back(std::move(ref));
		}
	}
}

void PlayerState_MotionBase::JsonSave(nlohmann::json& _js) const
{
	const char* key = GetStateJsonKey();
	if (!_js.contains(key)) _js[key] = nlohmann::json::object();
	m_params.SaveJson(_js[key]);

	const char* effKey = GetEffectsJsonKey();
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_effects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	_js[effKey] = std::move(arr);
}