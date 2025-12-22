#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/PlayerState_AttackBase.h"

class PlayerState_Attack : public PlayerState_AttackBase
{
public:
	PlayerState_Attack() = default;
	~PlayerState_Attack() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_Attack*>(&_proto))
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

	void OnUpdateAfterDash(Player* _owner) override;

	// この攻撃ステートのSEを指定
	const char* GetAttackSE() const override { return "Asset/Sound/Player/Attack.wav"; }

	// 差分ポイント
	const char* GetAnimationName() const override { return "Attack"; }
	const char* GetStateJsonKey() const override { return "PlayerState_Attack"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_Attack_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_Attack_Effect"; }

private:

	// カメラ演出用（このステート固有）
	Math::Vector3 m_cameraTargetOffset = Math::Vector3::Zero;
	Math::Vector3 m_cameraBossTargetOffset = Math::Vector3::Zero;
};