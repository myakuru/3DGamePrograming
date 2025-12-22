#include "PlayerState_AttackBase.h"
#include "Application/main.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_BackWordAvoid/PlayerState_BackWordAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoid/PlayerState_ForwardAvoid.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_FowardAvoidFast/PlayerState_ForwardAvoidFast.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_SpecialAttackCutIn/PlayerState_SpecialAttackCutIn.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevel0/PlayerState_ChargeLevel0.h"
#include "Application/GameObject/Character/Player/PlayerConfig.h"

void PlayerState_AttackBase::StateStart(Player* _owner)
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
	for (const auto& ref : m_effects)
	{ 
		if (auto e = ref->GetEffectBase().lock())
		{
			e->PlayForTarget<Player>(std::static_pointer_cast<Player>(_owner->GetMyAdls()));
		}
	}
	
	// 攻撃SE再生
	if (const char* se = GetAttackSE()) { KdAudioManager::Instance().Play(se, false)->SetVolume(1.0f); }

	// 当たり判定リセット
	_owner->ResetAttackCollision();

	m_time = 0.0f;
	m_lButtonKeyInput = false;
}

void PlayerState_AttackBase::StateUpdate(Player* _owner)
{
	PlayerStateServices::UpdateKatanaPos(_owner);
	const float deltaTime = Application::Instance().GetDeltaTime();
	if (m_attackDirection != Math::Vector3::Zero) PlayerStateServices::UpdateFacingDirect(_owner, m_attackDirection);

	m_animeTime = _owner->GetAnimator()->GetPlayProgress();

	// 攻撃当たり判定更新
	_owner->UpdateAttackCollision
	(
		m_params.attackRadius, m_params.attackDistance, m_params.attackCount, m_params.attackInterval,
		m_params.cameraShake, m_params.cameraTime, m_params.attackStartTime, m_params.attackEndTime
	);

	_owner->UpdateMoveDirectionFromInput();
	PlayerInputServices::TryAvoid<PlayerState_ForwardAvoidFast, PlayerState_BackWordAvoid, PlayerState_ForwardAvoid>(_owner, this, m_rButtonKeyInput);
	PlayerInputServices::TrySpecialAttack<PlayerState_SpecialAttackCutIn>(_owner, this);
	PlayerInputServices::TryESkill<PlayerState_ChargeLevel0>(_owner, this);

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_LBUTTON)) m_lButtonKeyInput = true;

	if (m_time < m_params.dashSpeedTime)
	{
		_owner->SetIsMoving(m_attackDirection * m_params.dashSpeed);
		m_time += deltaTime;
	}
	else
	{
		if (m_animeTime >= m_params.changeStateTime)
		{
			OnUpdateAfterDash(_owner);
		}
	}
}

void PlayerState_AttackBase::StateEnd(Player* _owner)
{
	for (const auto& ref : m_effects) { if (auto e = ref->GetEffectBase().lock()) e->StopEffect(); }
	OnEndExtra(_owner);
}

void PlayerState_AttackBase::ExposeParametersImGui()
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

void PlayerState_AttackBase::JsonInput(const nlohmann::json& _js)
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

void PlayerState_AttackBase::JsonSave(nlohmann::json& _js) const
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
