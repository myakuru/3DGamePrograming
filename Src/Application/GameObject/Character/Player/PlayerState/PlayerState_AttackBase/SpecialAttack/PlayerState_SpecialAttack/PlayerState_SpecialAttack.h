#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/PlayerState_AttackBase.h"

class PlayerState_SpecialAttack : public PlayerState_AttackBase
{
public:
	PlayerState_SpecialAttack() = default;
	~PlayerState_SpecialAttack() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_SpecialAttack*>(&_proto))
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

	// 差分ポイント
	const char* GetAnimationName() const override { return "PowerAttack"; }
	const char* GetStateJsonKey() const override { return "PlayerState_SpecialAttack"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_SpecialAttack_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_SpecialAttack_Effect"; }

	Math::Vector3 m_lastCameraPos = Math::Vector3::Zero;

};