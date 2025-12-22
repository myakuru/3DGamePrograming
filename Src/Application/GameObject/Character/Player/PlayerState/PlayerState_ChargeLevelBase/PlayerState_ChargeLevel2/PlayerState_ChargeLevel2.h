#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevelBase.h"

class PlayerState_ChargeLevel2 : public PlayerState_ChargeLevelBase
{
public:
	PlayerState_ChargeLevel2() = default;
	~PlayerState_ChargeLevel2() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_ChargeLevel2*>(&_proto))
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

	void OnUpdateAfterDash(Player* _owner)	override;

	// この攻撃ステートのアニメーション名などを指定
	const char* GetAnimationName() const override { return "ChargeAttack1"; }
	const char* GetStateJsonKey() const override { return "ChargeLevel2"; }
	const char* GetEffectsJsonKey() const override { return "ChargeLevel2Effects"; }
	const char* GetImGuiEffectLabel() const override { return "Charge Level 2 Effects"; }

};