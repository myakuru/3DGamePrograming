#pragma once
#include "MyFramework/State/StateBase/StateBase.h"
#include "MyFramework/State/Support/StateConfig.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateServices.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerInputServices.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateCommonParams.h"
#include "Application/GameObject/Utility/EffectReference.h"

class PlayerState_AvoidBase : public StateBase<Player>
{
public:
	PlayerState_AvoidBase() = default;
	~PlayerState_AvoidBase() override = default;

	// 差分ポイント
	virtual const char* GetAnimationName() const = 0;
	virtual const char* GetAttackSE() const				{ return nullptr; }

	//攻撃時、アニメーション開始直後の更新
	virtual void OnStartExtra(Player* _owner)			{ (void)_owner; }

	//攻撃時、前進し終わった後の更新
	virtual void OnUpdateAfterDash(Player* _owner)		{ (void)_owner; }
	virtual void OnEndExtra(Player* _owner)				{ (void)_owner; }

	// ステートのライフサイクル
	void StateStart(Player* _owner) override;
	void StateUpdate(Player* _owner) override;
	void StateEnd(Player* _owner) override;

	// ImGuiインスペクター表示
	void ExposeParametersImGui() override;

protected:

	StateParameter m_params;
	std::vector<std::shared_ptr<EffectReference>> m_effects;
	StateConfig<StateParameter> m_cfg;

	bool m_afterImagePlayed = false;
	float m_time = 0.0f;

	Math::Vector3 m_startCameraOffset = { 0.0f,1.0f,-4.5f };
	Math::Vector3 m_startBossCameraOffset = { 0.0f,1.0f,-7.5f };
	Math::Vector3 m_justAvoidCameraOffset = { 0.0f,0.7f,-1.2f };
	Math::Vector3 m_endCameraOffset = { 0.0f,1.0f,-3.0f };

};