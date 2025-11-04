#pragma once
#include"../PlayerState.h"
class PlayerState_BackWordAvoid :public PlayerStateBase
{
public:
	PlayerState_BackWordAvoid() = default;
	~PlayerState_BackWordAvoid() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	float m_chargeTime = 0.0f; // チャージ時間

	bool m_isCharging = false; // チャージ中かどうか
	bool m_isCharged = false; // チャージが完了したかどうか

	bool m_afterImagePlayed = false; // 残像エフェクトが再生されたかどうか


	bool m_justAvoided = false; // ジャスト回避が成功したかどうか

};