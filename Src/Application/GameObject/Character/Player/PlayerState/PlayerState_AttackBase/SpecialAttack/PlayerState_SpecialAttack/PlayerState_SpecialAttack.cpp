#include "PlayerState_SpecialAttack.h"
#include "Application/GameObject/Character/Player/PlayerState/PlayerState_AttackBase/SpecialAttack/PlayerState_SpecialAttackEnd/PlayerState_SpecialAttackEnd.h"

void PlayerState_SpecialAttack::OnUpdateAfterDash(Player* _owner)
{
	// アニメーション時間の取得
	m_animeTime = _owner->GetAnimator()->GetPlayProgress();

	if (m_animeTime >= m_params.changeStateTime)
	{
		// カメラの位置をImGUiで変更する
		if (auto camera = _owner->GetPlayerCamera().lock())
		{
			camera->SetTargetLookAt(m_lastCameraPos);
		}

		PlayerInputServices::TryAnimationEnd<PlayerState_SpecialAttackEnd>(_owner, this);
	}

	Math::Vector3 toEnemyDir = m_nearestEnemyPos - _owner->GetPos();
	if (auto camera = _owner->GetPlayerCamera().lock())
	{
		toEnemyDir.Normalize();
		const float yawRad = std::atan2(toEnemyDir.x, toEnemyDir.z);
		const float yawDeg = DirectX::XMConvertToDegrees(yawRad);
		camera->SetPlayerRotation({ 0.0f, yawDeg , 0.0f });
	}

	_owner->SetIsMoving(Math::Vector3::Zero);
}