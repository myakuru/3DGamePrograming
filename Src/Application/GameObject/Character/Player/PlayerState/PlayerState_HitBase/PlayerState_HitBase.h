#pragma once
#include "MyFramework/State/StateBase/StateBase.h"
#include "MyFramework/State/Support/StateConfig.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateServices.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerInputServices.h"
#include "Application/GameObject/Character/Player/PlayerState/Common/PlayerStateCommonParams.h"
#include "Application/GameObject/Utility/EffectReference.h"

class PlayerState_HitBase : public StateBase<Player>
{
public:
	PlayerState_HitBase() = default;
	~PlayerState_HitBase() override = default;

	// 差分ポイント
	virtual const char* GetAnimationName() const = 0;
	virtual const char* GetStateJsonKey() const = 0;
	virtual const char* GetEffectsJsonKey() const = 0;
	virtual const char* GetImGuiEffectLabel() const = 0;

	// オプション差分
	virtual bool IsInvincibleDuringHit() const { return false; }
	virtual bool EndOnAnimationFinished() const { return true; } // アニメ終了で抜ける
	virtual float GetStunDuration() const { return m_params.changeStateTime; } // 既存パラメータ流用
	virtual Math::Vector3 GetKnockback() const { return m_knockback; }

	// フック
	virtual void OnStartExtra(Player* owner) { (void)owner; }
	virtual void OnUpdateExtra(Player* owner, float dt) { (void)owner; (void)dt; }
	virtual void OnEndExtra(Player* owner) { (void)owner; }

	// ライフサイクル
	void StateStart(Player* owner) override;
	void StateUpdate(Player* owner) override;
	void StateEnd(Player* owner) override;

	// 設定UI/JSON
	void ExposeParametersImGui() override;
	void JsonInput(const nlohmann::json& js) override;
	void JsonSave(nlohmann::json& js) const override;

protected:
	// 共通パラメータ（blendTime, animationSpeed 等を含む）
	StateParameter m_params{};
	StateConfig<StateParameter> m_cfg{};

	// エフェクト
	std::vector<std::shared_ptr<EffectReference>> m_effects;

	// タイマー
	float m_time = 0.0f;

	// ノックバック
	Math::Vector3 m_knockback = Math::Vector3::Zero;

	// カメラ揺れなど拡張用
};