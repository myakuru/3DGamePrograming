#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevelBase.h"

class PlayerState_ChargeLevel0 : public PlayerState_ChargeLevelBase
{
public:
	PlayerState_ChargeLevel0() = default;
	~PlayerState_ChargeLevel0() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_ChargeLevel0*>(&_proto))
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
	const char* GetAnimationName() const override { return "ChargeAttack0"; }
	const char* GetStateJsonKey() const override { return "ChargeLevel0"; }
	const char* GetEffectsJsonKey() const override { return "ChargeLevel0Effects"; }
	const char* GetImGuiEffectLabel() const override { return "Charge Level 0 Effects"; }

};