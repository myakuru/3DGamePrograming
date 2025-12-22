#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_ChargeLevelBase/PlayerState_ChargeLevelBase.h"

class PlayerState_ChargeLevel1 : public PlayerState_ChargeLevelBase
{
public:
	PlayerState_ChargeLevel1() = default;
	~PlayerState_ChargeLevel1() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_ChargeLevel1*>(&_proto))
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
	const char* GetStateJsonKey() const override { return "ChargeLevel1"; }
	const char* GetEffectsJsonKey() const override { return "ChargeLevel1Effects"; }
	const char* GetImGuiEffectLabel() const override { return "Charge Level 1 Effects"; }

};