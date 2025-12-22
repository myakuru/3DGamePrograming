#pragma once
#include "MyFramework/State/StateBase/StateBase.h"
#include "MyFramework/State/Support/StateConfig.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateCommonParams.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateServices.h"
#include "Application/GameObject/Utility/EffectReference.h"

class PlayerState_MotionBase : public StateBase<Player>
{
public:
	PlayerState_MotionBase() = default;
	~PlayerState_MotionBase() override = default;

	// 差分ポイント
	virtual const char* GetAnimationName() const = 0;
	virtual const char* GetMoveSE() const { return nullptr; }
	virtual void OnStartExtra(Player* _owner) { (void)_owner; }
	virtual void OnUpdateExtra(Player* _owner, float deltaTime) { (void)_owner; (void)deltaTime; }
	virtual void OnEndExtra(Player* _owner) { (void)_owner; }

	virtual const char* GetStateJsonKey() const = 0;
	virtual const char* GetEffectsJsonKey() const = 0;
	virtual const char* GetImGuiEffectLabel() const = 0;

	// ライフサイクル
	void StateStart(Player* _owner) override;
	void StateUpdate(Player* _owner) override;
	void StateEnd(Player* _owner) override;

	// 設定UI/JSON
	void ExposeParametersImGui() override;
	void JsonInput(const nlohmann::json& _js) override;
	void JsonSave(nlohmann::json& _js) const override;

protected:
	StateParameter m_params{};
	StateConfig<StateParameter> m_cfg{};
	std::vector<std::shared_ptr<EffectReference>> m_effects;

	float m_time = 0.0f;
};