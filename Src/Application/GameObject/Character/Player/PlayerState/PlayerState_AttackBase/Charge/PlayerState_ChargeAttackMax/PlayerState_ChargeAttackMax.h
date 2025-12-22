#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/Charge/PlayerState_ChargeBase.h"

class PlayerState_ChargeAttackMax : public PlayerState_ChargeBase
{
public:
	PlayerState_ChargeAttackMax() = default;
	~PlayerState_ChargeAttackMax() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_ChargeAttackMax*>(&_proto))
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

	void ExposeParametersImGui() override;
	void JsonInput(const nlohmann::json& _js) override;
	void JsonSave(nlohmann::json& _js) const override;

	// 差分ポイント
	const char* GetAnimationName() const override { return "Eskill"; }
	const char* GetStateJsonKey() const override { return "PlayerState_ChargeAttackMax"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_ChargeAttackMax_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_ChargeAttackMax_Effect"; }

	// この攻撃ステートのSEを指定
	const char* GetAttackSE() const override { return "Asset/Sound/Player/Attack4.WAV"; }

	// 派生先での追加処理
	void OnStartExtra(Player* _owner) override;
	void OnUpdateAfterDash(Player* _owner) override;

};