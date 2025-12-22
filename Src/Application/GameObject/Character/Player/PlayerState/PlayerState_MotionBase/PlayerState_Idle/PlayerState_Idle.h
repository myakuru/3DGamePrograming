#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_MotionBase.h"

class PlayerState_Idle : public PlayerState_MotionBase
{
public:
	PlayerState_Idle() = default;
	~PlayerState_Idle() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_Idle*>(&_proto))
		{
			// プロトタイプの差分パラメータのみ反映
			m_params = p->m_params;

			// エフェクトも反映
			m_effects = p->m_effects;

			return true;
		}
		return false;
	}

private:

	// 追加フック
	void OnStartExtra(Player* owner) override;
	void OnUpdateExtra(Player* owner, float deltaTime) override;

	// 差分ポイント
	const char* GetAnimationName() const override { return "Idle"; }
	const char* GetStateJsonKey() const override { return "PlayerState_Idle"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_Idle_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_Idle_Effect"; }

	// 旧コードで使用していたカメラターゲットオフセット
	Math::Vector3 m_cameraTargetOffset = StateParameter::kCameraTargetOffset;
	Math::Vector3 m_cameraBossTargetOffset = StateParameter::kCameraBossTargetOffset;
};