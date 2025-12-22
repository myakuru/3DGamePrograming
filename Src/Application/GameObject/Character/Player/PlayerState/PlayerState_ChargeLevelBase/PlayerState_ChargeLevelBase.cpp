#include "PlayerState_ChargeLevelBase.h"
#include "Application/main.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Character/Player/PlayerConfig.h"

void PlayerState_ChargeLevelBase::StateStart(Player* _owner)
{
	if (_owner && _owner->GetPlayerConfig())
	{
		_owner->GetPlayerConfig()->ApplyPrototypeParametersTo(*this);
	}

	auto anime = _owner->GetAnimeModel()->GetAnimation(GetAnimationName());
	_owner->GetAnimator()->SetAnimation(anime, m_params.blendTime, false);
	_owner->SetAnimeSpeed(m_params.animationSpeed);
	OnStartExtra(_owner);

	// 敵方向に向く処理
	PlayerStateServices::UpdateFocusAndNearest
	(
		_owner,
		m_params.searchEnemyRadius,
		m_focusTarget,
		m_nearestEnemy,
		m_nearestEnemyPos,
		m_minDistSq,
		m_focusRemainSec,
		m_focusDurationSec
	);

	// m_attackDirectionに敵の方向をセット
	m_attackDirection = PlayerStateServices::DecideAttackDirectionAndRotate(_owner, m_nearestEnemy, m_nearestEnemyPos);

	// エフェクト再生
	for (const auto& ref : m_effects) { if (auto e = ref->GetEffectBase().lock()) e->PlayForTarget<Player>(std::static_pointer_cast<Player>(_owner->GetMyAdls())); }

	// 攻撃SE再生
	if (const char* se = GetAttackSE()) { KdAudioManager::Instance().Play(se, false)->SetVolume(1.0f); }

	// 無敵化
	_owner->SetInvincible(true);

	// ガードブレイク状態解除
	_owner->SetGuardBreak(true);

	m_time = 0.0f;
	m_lButtonKeyInput = false;
}

void PlayerState_ChargeLevelBase::StateUpdate(Player* _owner)
{
	// アニメーション時間のデバッグ表示
	m_animeTime = _owner->GetAnimator()->GetPlayProgress();

	// スローモーション処理
	if (m_animeTime >= m_params.startSlowMotionTime && m_animeTime <= m_params.endSlowMotionTime)
	{
		Application::Instance().SetFpsScale(0.1f);
	}
	else
	{
		Application::Instance().SetFpsScale(1.0f);
	}

	// 刀は鞘の中にある状態
	PlayerStateServices::UpdateUnsheathed(_owner);

	// 攻撃中の移動方向で回転を更新
	if (_owner->GetMovement() != Math::Vector3::Zero)
	{
		Math::Vector3 moveDir = _owner->GetMovement();
		moveDir.y = 0.0f;
		moveDir.Normalize();
		_owner->UpdateQuaternionDirect(moveDir);
	}

	// 0.5f
	if (m_animeTime >= m_params.changeStateTime)
	{
		OnUpdateAfterDash(_owner);
	}

	// 移動を止める
	_owner->SetIsMoving(Math::Vector3::Zero);
}

void PlayerState_ChargeLevelBase::StateEnd(Player* _owner)
{
	for (const auto& ref : m_effects) { if (auto e = ref->GetEffectBase().lock()) e->StopEffect(); }
	OnEndExtra(_owner);
}

void PlayerState_ChargeLevelBase::ExposeParametersImGui()
{
	// StateParameter を通常表示
	m_params.ExposeImGui();

	// 複数エフェクトの編集UI（派生が提供するラベルを使用）
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

void PlayerState_ChargeLevelBase::JsonInput(const nlohmann::json& _js)
{
	// ステート固有ノード配下から StateParameter を読み込み
	const char* stateKey = GetStateJsonKey();
	if (_js.contains(stateKey))
	{
		m_params.LoadJson(_js[stateKey]);
	}
	else
	{
		// ルート直下にある場合も許容
		m_params.LoadJson(_js);
	}

	// エフェクト配列の読み込み
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

void PlayerState_ChargeLevelBase::JsonSave(nlohmann::json& _js) const
{
	// ステート固有キー配下に StateParameter を保存
	const char* stateKey = GetStateJsonKey();
	if (!_js.contains(stateKey)) _js[stateKey] = nlohmann::json::object();
	m_params.SaveJson(_js[stateKey]);

	// エフェクト配列を保存
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
