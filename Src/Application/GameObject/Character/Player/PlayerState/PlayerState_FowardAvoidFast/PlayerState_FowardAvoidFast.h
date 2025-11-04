#pragma once
#include"../PlayerState.h"
class PlayerState_FowardAvoidFast :public PlayerStateBase
{
public:
	PlayerState_FowardAvoidFast() = default;
	~PlayerState_FowardAvoidFast() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	bool m_afterImagePlayed = false; // 残像エフェクトが再生されたかどうか

	bool m_justAvoided = false; // ジャスト回避が成功したかどうか

};