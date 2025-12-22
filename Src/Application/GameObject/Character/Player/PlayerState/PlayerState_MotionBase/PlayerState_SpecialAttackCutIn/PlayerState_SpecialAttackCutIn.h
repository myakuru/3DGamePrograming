#pragma once
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_MotionBase/PlayerState_MotionBase.h"

class PlayerState_SpecialAttackCutIn : public PlayerState_MotionBase
{
public:
	PlayerState_SpecialAttackCutIn() = default;
	~PlayerState_SpecialAttackCutIn() override = default;

	bool TryApplyFromPrototype(const StateBase<Player>& _proto) override
	{
		if (const auto* p = dynamic_cast<const PlayerState_SpecialAttackCutIn*>(&_proto))
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
	// 追加処理フック
	void OnStartExtra(Player* _owner) override;
	void OnUpdateExtra(Player* _owner, float deltaTime) override;
	void OnEndExtra(Player* _owner) override;

	// MotionBase 差分ポイント
	const char* GetAnimationName() const override { return "CutIn"; }
	const char* GetMoveSE() const override { return nullptr; } // 必要なら移動SEを指定
	const char* GetStateJsonKey() const override { return "PlayerState_SpecialAttackCutIn"; }
	const char* GetEffectsJsonKey() const override { return "PlayerState_SpecialAttackCutIn_Effects"; }
	const char* GetImGuiEffectLabel() const override { return "PlayerState_SpecialAttackCutIn_Effect"; }

	// このステート固有のパラメータ
	float m_cameraStartRotationSmooth = 5.0f;
	float m_cameraStartDistanceSmooth = 5.0f;
	Math::Vector3 m_cameraCutInOffset = { 0.0f, 0.6f, -1.7f };
	Math::Vector3 m_cameraCutInRotation = { 10.0f, 90.0f, 5.0f };
	float m_cutInCameraTime = 0.6f;

	// 追従のために使用
	Math::Vector3 m_nearestEnemyPos = Math::Vector3::Zero;

	// カメラの終了時スムーズ（元のコードの復帰用。必要なら外から設定）
	float m_cameraRotationSmooth = 5.0f;
	float m_cameraDistanceSmooth = 5.0f;

	// Debug表示用
	float m_animeTime = 0.0f;
};