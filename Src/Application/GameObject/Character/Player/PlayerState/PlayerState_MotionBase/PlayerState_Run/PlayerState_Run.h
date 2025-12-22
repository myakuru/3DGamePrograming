#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_MotionBase.h"

class PlayerState_Run : public PlayerState_MotionBase
{
public:
	PlayerState_Run() = default;
	~PlayerState_Run() override = default;

	void StateUpdate(Player* _owner) override;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_Run*>(&_proto))
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
	const char* GetAnimationName() const override { return "Run"; }
	const char* GetStateJsonKey() const override { return "PlayerState_Run"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_Run_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_Run_Effect"; }
	const char* GetMoveSE() const override { return "Asset/Sound/Player/Run.wav"; }

	// 追加フック
	void OnStartExtra(Player* owner) override;
	void OnUpdateExtra(Player* owner, float deltaTime) override;
	void OnEndExtra(Player* owner) override;

	std::shared_ptr<KdSoundInstance> m_runSound = nullptr;

};