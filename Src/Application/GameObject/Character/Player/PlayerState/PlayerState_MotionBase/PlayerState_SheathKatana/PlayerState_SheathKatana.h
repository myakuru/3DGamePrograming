#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_MotionBase.h"

class PlayerState_SheathKatana : public PlayerState_MotionBase
{
public:
	PlayerState_SheathKatana() = default;
	~PlayerState_SheathKatana() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_SheathKatana*>(&_proto))
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

	// 差分ポイント
	const char* GetAnimationName() const override { return "SheathKatana"; }
	const char* GetStateJsonKey() const override { return "PlayerState_SheathKatana"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_SheathKatana_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_SheathKatana_Effect"; }

	const char* GetMoveSE() const override { return nullptr; }

	// フック
	void OnUpdateExtra(Player* owner, float deltaTime) override;
};