#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_HitBase/PlayerState_HitBase.h"

class PlayerState_Hit : public PlayerState_HitBase
{
public:
	PlayerState_Hit() = default;
	~PlayerState_Hit() override = default;

	// 差分ポイント
	const char* GetAnimationName() const override { return "Hit"; }
	const char* GetStateJsonKey() const override { return "PlayerState_Hit"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_Hit_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_Hit_Effect"; }

	// オプション
	bool IsInvincibleDuringHit() const override { return false; } // のけぞり中の無敵が必要なら true
	bool EndOnAnimationFinished() const override { return true; } // アニメ終了で復帰

protected:
	void OnStartExtra(Player* owner) override;
	void OnUpdateExtra(Player* owner, float dt) override;
	void OnEndExtra(Player* owner) override;

private:
	// 追加演出用（カメラシェイク等）
	Math::Vector2 m_cameraShakePower = { 0.03f, 0.03f };
	float m_cameraShakeTime = 0.15f;
};