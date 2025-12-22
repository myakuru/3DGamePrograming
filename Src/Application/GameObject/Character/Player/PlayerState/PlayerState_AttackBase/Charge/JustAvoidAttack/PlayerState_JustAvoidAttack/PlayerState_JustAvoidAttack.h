#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/Charge/PlayerState_ChargeBase.h"

class PlayerState_JustAvoidAttack : public PlayerState_ChargeBase
{
public:
	PlayerState_JustAvoidAttack() = default;
	~PlayerState_JustAvoidAttack() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_JustAvoidAttack*>(&_proto))
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

	// 派生先での追加処理
	void OnUpdateAfterDash(Player* _owner) override;
	void OnStartExtra(Player* _owner) override;

	// 差分ポイント
	const char* GetAnimationName() const override { return "Eskill"; }
	const char* GetStateJsonKey() const override { return "PlayerState_JustAvoidAttack"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_JustAvoidAttack_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_JustAvoidAttack_Effect"; }

	// この攻撃ステートのSEを指定
	const char* GetAttackSE() const override { return "Asset/Sound/Player/JustAttack.WAV"; }

};