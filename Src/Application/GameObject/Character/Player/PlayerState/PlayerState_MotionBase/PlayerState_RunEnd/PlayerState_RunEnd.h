#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_MotionBase.h"

class PlayerState_RunEnd : public PlayerState_MotionBase
{
public:
	PlayerState_RunEnd() = default;
	~PlayerState_RunEnd() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_RunEnd*>(&_proto))
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
	const char* GetAnimationName() const override { return "RunEnd"; }
	const char* GetStateJsonKey() const override { return "PlayerState_RunEnd"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_RunEnd_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_RunEnd_Effect"; }
	const char* GetMoveSE() const override { return nullptr; }

	// フック
	void OnStartExtra(Player* owner) override;
	void OnUpdateExtra(Player* owner, float deltaTime) override;
};