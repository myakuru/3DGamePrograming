#include "PlayerState_ChargeBase.h"

void PlayerState_ChargeBase::StateUpdate(Player* _owner)
{
	// 刀を手に持たせる
	PlayerStateServices::UpdateKatanaPos(_owner);

	// 敵の方向に向く処理
	if (m_attackDirection != Math::Vector3::Zero) PlayerStateServices::UpdateFacingDirect(_owner, m_attackDirection);

	// 攻撃当たり判定更新
	_owner->UpdateAttackCollision
	(
		m_params.attackRadius, m_params.attackDistance, m_params.attackCount, m_params.attackInterval,
		m_params.cameraShake, m_params.cameraTime, m_params.attackStartTime, m_params.attackEndTime
	);


	// 先行ダッシュ処理
	PlayerStateServices::PreDashTowardEnemy
	(
		_owner,
		m_nearestEnemyPos,
		m_attackDirection,
		m_overshootDist,
		1.0f,
		10.0f
	);

	// ダッシュ後の遷移処理
	OnUpdateAfterDash(_owner);

}
