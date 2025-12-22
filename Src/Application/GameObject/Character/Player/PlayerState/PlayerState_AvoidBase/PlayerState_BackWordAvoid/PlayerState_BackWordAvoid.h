#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AvoidBase/PlayerState_AvoidBase.h"

class PlayerState_BackWordAvoid : public PlayerState_AvoidBase
{
public:
	PlayerState_BackWordAvoid() = default;
	~PlayerState_BackWordAvoid() override = default;

	const char* GetAnimationName() const override { return "AvoidBackward"; }

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_BackWordAvoid*>(&_proto))
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

	void OnStartExtra(Player* _owner) override;

};