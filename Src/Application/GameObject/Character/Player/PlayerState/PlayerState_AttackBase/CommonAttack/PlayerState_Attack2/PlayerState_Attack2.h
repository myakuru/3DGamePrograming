#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/PlayerState_AttackBase.h"

class PlayerState_Attack2 : public PlayerState_AttackBase
{
public:
	PlayerState_Attack2() = default;
	~PlayerState_Attack2() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_Attack2*>(&_proto))
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

	void JsonInput(const nlohmann::json& _js) override;
	void JsonSave(nlohmann::json& _js) const override;

	// 差分ポイント
	const char* GetAnimationName() const override { return "Attack2"; }
	const char* GetStateJsonKey() const override { return "PlayerState_Attack2"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_Attack2_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_Attack2_Effect"; }

	// この攻撃ステートのSEを指定
	const char* GetAttackSE() const override { return "Asset/Sound/Player/Attack2.WAV"; }

};