#pragma once
#include "MyFramework/State/StateBase/StateBase.h"
#include "MyFramework/State/Support/StateConfig.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateServices.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerInputServices.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateCommonParams.h"
#include "Application/GameObject/Utility/EffectReference.h"

class PlayerState_AttackBase : public StateBase<Player>
{
public:
	PlayerState_AttackBase() = default;
	~PlayerState_AttackBase() override = default;

	// 差分ポイント
	virtual const char* GetAnimationName() const		= 0;
	virtual const char* GetAttackSE() const				{ return nullptr; }
	//攻撃時、アニメーション開始直後の更新
	virtual void OnStartExtra(Player* _owner)			{ (void)_owner; }
	//攻撃時、前進し終わった後の更新
	virtual void OnUpdateAfterDash(Player* _owner)		{ (void)_owner; }
	virtual void OnEndExtra(Player* _owner)				{ (void)_owner; }

	virtual const char* GetStateJsonKey() const			= 0;
	virtual const char* GetEffectsJsonKey() const		= 0;
	virtual const char* GetImGuiEffectLabel() const		= 0;

	// ステートのライフサイクル
	void StateStart(Player* _owner) override;
	void StateUpdate(Player* _owner) override;
	void StateEnd(Player* _owner) override;

	// ImGuiインスペクター表示
	void ExposeParametersImGui() override;
	void JsonInput(const nlohmann::json& _js) override;
	void JsonSave(nlohmann::json& _js) const override;

	StateParameter GetStateParameter() const { return m_params; }

protected:

	StateParameter m_params{};
	std::vector<std::shared_ptr<EffectReference>> m_effects;

	StateConfig<StateParameter> m_cfg;

	Math::Vector3 m_attackDirection = Math::Vector3::Zero;
	bool m_lButtonKeyInput = false;
	bool m_rButtonKeyInput = false;
	float m_time = 0.0f;
	float m_animeTime = 0.0f;

	//////
	std::weak_ptr<KdGameObject>			m_focusTarget;        //    フォーカスターゲット
	std::shared_ptr<KdGameObject>		m_nearestEnemy;
	Math::Vector3                    m_nearestEnemyPos = Math::Vector3::Zero;
	float                            m_minDistSq = std::numeric_limits<float>::max();
	float m_focusRemainSec = 0.0f;                    //    フォーカスタイマー
	const float m_focusDurationSec = 10.0f;            // フォーカス継続時間(調整用)
};