#pragma once
#include"../PlayerState.h"
class ShineEffectBlue;
class GroundFreezes;
class PlayerState_ChargeAttack1 :public PlayerStateBase
{
public:
	PlayerState_ChargeAttack1() = default;
	~PlayerState_ChargeAttack1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	float m_chargeTime = 0.0f; // チャージ時間

	bool m_isCharging = false; // チャージ中かどうか
	bool m_isCharged = false; // チャージが完了したかどうか

	std::weak_ptr<ShineEffectBlue> m_shineEffect; // エフェクト
	std::weak_ptr<GroundFreezes> m_groundFreezes; // エフェクト

	int m_chargeAttackCount = 0;      // 何回呼んだか

	bool m_flag = false; // 攻撃フラグ

};