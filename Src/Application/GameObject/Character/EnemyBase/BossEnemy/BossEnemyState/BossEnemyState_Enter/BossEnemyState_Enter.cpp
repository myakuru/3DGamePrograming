#include "BossEnemyState_Enter.h"
#include"../BossEnemyState_Idle/BossEnemyState_Idle.h"
#include"Application/Scene/SceneManager.h"

#include "Application/GameObject/Utility/EffectReference.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"

void BossEnemyState_Enter::StateStart(BossEnemy* _owner)
{
	auto anime = _owner->GetAnimeModel()->GetAnimation("Anim_Enter");
	_owner->GetAnimator()->SetAnimation(anime, m_stateParameter.blendTime, false);
	BossEnemyStateBase::StateStart(_owner);
	// アニメーション速度を変更
	_owner->SetAnimeSpeed(m_stateParameter.animationSpeed);

	m_effectPlayed = false;

	if (SceneManager::Instance().GetCurrentScene()->GetSceneName() != "Title")
	{
		KdAudioManager::Instance().Play("Asset/Sound/BossEnemy/WaterEnter.WAV", false)->SetVolume(1.0f);
	}
}

void BossEnemyState_Enter::StateUpdate(BossEnemy* _owner)
{
	// アニメーションの再生時間を取得
	m_animeTime = _owner->GetAnimator()->GetPlayProgress();

	if (!m_effectPlayed)
	{
		// エフェクト再生・移動停止（複数）
		for (const auto& ref : m_enemyEffects)
		{
			if (auto effect = ref->GetEffectBase().lock())
			{
				effect->PlayForTarget<BossEnemy>(std::static_pointer_cast<BossEnemy>(_owner->GetMyAdls()));
			}
		}
	}

	if (_owner->GetAnimator()->IsAnimationEnd())
	{
		auto state = std::make_shared<BossEnemyState_Idle>();
		_owner->ChangeState(state);
		return;
	}

	_owner->SetIsMoving(Math::Vector3::Zero);
}

void BossEnemyState_Enter::StateEnd(BossEnemy* _owner)
{
	for (const auto& ref : m_enemyEffects)
	{
		if (auto effect = ref->GetEffectBase().lock())
		{
			effect->StopEffect();
		}
	}
}

void BossEnemyState_Enter::ApplyFromConfig(const BossEnemyStateBase& other)
{
	assert(typeid(other) == typeid(BossEnemyState_Enter));
	const auto& p = static_cast<const BossEnemyState_Enter&>(other);
	m_stateParameter.blendTime = p.m_stateParameter.blendTime;
	m_stateParameter.animationSpeed = p.m_stateParameter.animationSpeed;
	m_enemyEffects = p.m_enemyEffects;
}

void BossEnemyState_Enter::ExposeParametersImGui()
{
	ImGui::Text("Effects");
	for (size_t i = 0; i < m_enemyEffects.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		m_enemyEffects[i]->ImGuiInspector("EffectSelection");
		ImGui::PopID();
	}
	if (ImGui::SmallButton("+")) { m_enemyEffects.emplace_back(std::make_shared<EffectReference>()); }
	if (ImGui::SmallButton("-")) { if (!m_enemyEffects.empty()) m_enemyEffects.pop_back(); }

	ImGui::DragFloat(U8("アニメーションブレンド"), &m_stateParameter.blendTime);
	ImGui::DragFloat(U8("アニメーション速度"), &m_stateParameter.animationSpeed, 1.0f, 1.0f, 200.0f);
}

void BossEnemyState_Enter::LoadParametersJson(const nlohmann::json& js)
{
	// 複数のみ
	if (js.contains("BossEnemyState_Enter_Effects") && js["BossEnemyState_Enter_Effects"].is_array())
	{
		m_enemyEffects.clear();
		for (const auto& node : js["BossEnemyState_Enter_Effects"])
		{
			auto ref = std::make_shared<EffectReference>();
			ref->JsonInput("Effect", node);
			m_enemyEffects.emplace_back(std::move(ref));
		}
	}

	if (!js.contains("BossEnemyState_Enter")) return;
	const auto& stateNode = js["BossEnemyState_Enter"];
	if (stateNode.contains("BossEnemy"))
	{
		const auto& enemyNode = stateNode["BossEnemy"];
		if (enemyNode.contains("blendTime")) m_stateParameter.blendTime = enemyNode["blendTime"].get<float>();
		if (enemyNode.contains("animationSpeed")) m_stateParameter.animationSpeed = enemyNode["animationSpeed"].get<float>();
	}
}

void BossEnemyState_Enter::SaveParametersJson(nlohmann::json& js) const
{
	// 複数のみ
	nlohmann::json arr = nlohmann::json::array();
	for (const auto& ref : m_enemyEffects)
	{
		nlohmann::json item = nlohmann::json::object();
		ref->JsonSave("Effect", item);
		arr.push_back(item);
	}
	js["BossEnemyState_Enter_Effects"] = std::move(arr);

	if (!js.contains("BossEnemyState_Enter")) js["BossEnemyState_Enter"] = nlohmann::json::object();
	auto& stateNode = js["BossEnemyState_Enter"];

	stateNode["BossEnemy"]["blendTime"] = m_stateParameter.blendTime;
	stateNode["BossEnemy"]["animationSpeed"] = m_stateParameter.animationSpeed;
}
