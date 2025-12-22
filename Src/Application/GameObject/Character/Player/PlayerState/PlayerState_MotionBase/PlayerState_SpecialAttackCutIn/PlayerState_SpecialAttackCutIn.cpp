#include "PlayerState_SpecialAttackCutIn.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/SpecialAttack/PlayerState_SpecialAttack/PlayerState_SpecialAttack.h"
#include "Application/GameObject/Camera/PlayerCamera/PlayerCamera.h"
#include "Application/main.h"
#include "Application/GameObject/Effect/EffekseerEffect/EffekseerEffectBase.h"
#include "Application/GameObject/Utility/EffectReference.h"

void PlayerState_SpecialAttackCutIn::OnStartExtra(Player* _owner)
{
	// アニメ速度は MotionBase で m_params.animationSpeed を適用済み
	// カメラ初期設定
	if (auto camera = _owner->GetPlayerCamera().lock())
	{
		camera->SetTargetLookAt(m_cameraCutInOffset);
		camera->SetRotationSmooth(m_cameraStartRotationSmooth);
		camera->SetDistanceSmooth(m_cameraStartDistanceSmooth);
	}

	// 無敵
	_owner->SetInvincible(true);

	// 近接敵方向の初期取得（必要に応じてサービス層があればそれを利用）
	// ここではプレースホルダ: 最短敵座標はオーナーから取得できるインタフェースがある想定
	// 実プロジェクトの共通サービスがあるならそちらに差し替え
	m_nearestEnemyPos = _owner->GetPos(); // 初期値安全策
}

void PlayerState_SpecialAttackCutIn::OnUpdateExtra(Player* _owner, float /*deltaTime*/)
{
	// アニメ時間のデバッグ表示
	m_animeTime = _owner->GetAnimator()->GetPlayProgress();

	// 敵方向にカメラヨーを合わせる
	Math::Vector3 toEnemyDir = m_nearestEnemyPos - _owner->GetPos();
	if (auto camera = _owner->GetPlayerCamera().lock())
	{
		toEnemyDir.Normalize();
		const float yawRad = std::atan2(-toEnemyDir.x, -toEnemyDir.z);
		const float yawDeg = DirectX::XMConvertToDegrees(yawRad);
		camera->SetPlayerRotation({ 0.0f, yawDeg, 0.0f });
	}

	// カメラオフセット維持（時間経過での調整）
	if (m_animeTime >= 0.5f)
	{
		if (auto camera = _owner->GetPlayerCamera().lock())
		{
			camera->SetTargetLookAt(m_cameraCutInOffset);
		}
	}

	// 刀位置更新（共通ロジックを利用）
	PlayerStateServices::UpdateKatanaPos(_owner);

	// アニメ終了で必殺技状態へ
	if (_owner->GetAnimator()->IsAnimationEnd())
	{
		PlayerInputServices::TryChange<PlayerState_SpecialAttack>(this);
	}
}

void PlayerState_SpecialAttackCutIn::OnEndExtra(Player* _owner)
{
	// カメラ復帰
	if (auto camera = _owner->GetPlayerCamera().lock())
	{
		camera->SetRotationSmooth(m_cameraRotationSmooth);
		camera->SetDistanceSmooth(m_cameraDistanceSmooth);
	}
}