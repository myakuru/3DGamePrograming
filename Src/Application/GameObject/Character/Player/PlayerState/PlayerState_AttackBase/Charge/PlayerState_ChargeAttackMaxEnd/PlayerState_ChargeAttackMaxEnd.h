#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/PlayerState_AttackBase.h"

class PlayerState_ChargeAttackMaxEnd : public PlayerState_AttackBase
{
public:
	PlayerState_ChargeAttackMaxEnd() = default;
	~PlayerState_ChargeAttackMaxEnd() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_ChargeAttackMaxEnd*>(&_proto))
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

	void JsonInput(const nlohmann::json& _js) override;
	void JsonSave(nlohmann::json& _js) const override;

	// 派生先での追加処理
	void OnUpdateAfterDash(Player* _owner) override;
	void OnEndExtra(Player* _owner) override;

	// 差分ポイント
	const char* GetAnimationName() const override { return "ChargeMax"; }
	const char* GetStateJsonKey() const override { return "PlayerState_ChargeAttackMaxEnd"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_ChargeAttackMaxEnd_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_ChargeAttackMaxEnd_Effect"; }

	// この攻撃ステートのSEを指定
	const char* GetAttackSE() const override { return "Asset/Sound/Player/ChargeAttack.WAV"; }

};