#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/PlayerState_AttackBase.h"

class PlayerState_SpecialAttackEnd : public PlayerState_AttackBase
{
public:
	PlayerState_SpecialAttackEnd() = default;
	~PlayerState_SpecialAttackEnd() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_SpecialAttackEnd*>(&_proto))
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

	void OnUpdateAfterDash(Player* _owner) override;
	void OnEndExtra(Player* _owner) override;

	// この攻撃ステートのSEを指定
	const char* GetAttackSE() const override { return "Asset/Sound/Player/SpecialAttackEnd.WAV"; }

	// 差分ポイント
	const char* GetAnimationName() const override { return "ChargeAttack0"; }
	const char* GetStateJsonKey() const override { return "PlayerState_SpecialAttackEnd"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_SpecialAttackEnd_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_SpecialAttackEnd_Effect"; }

	float m_animeTime = 0.0f;
	Math::Vector3 m_lastCameraPos = Math::Vector3::Zero;

};