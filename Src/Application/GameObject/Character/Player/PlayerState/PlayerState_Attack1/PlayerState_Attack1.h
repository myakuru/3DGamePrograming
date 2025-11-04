#pragma once
#include"../PlayerState.h"
class Rotation;
class PlayerState_Attack1 : public PlayerStateBase
{
public:
	PlayerState_Attack1() = default;
	~PlayerState_Attack1() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	void UpdateEffect(); // エフェクト更新

	PlayerState_Attack1Parameter m_attackParam; // 攻撃パラメータ

	std::weak_ptr<Rotation> m_effect; // エフェクト

	bool m_flag = false; // 攻撃フラグ
	bool m_effectOnce = false; // エフェクト一回だけ再生するフラグ
};