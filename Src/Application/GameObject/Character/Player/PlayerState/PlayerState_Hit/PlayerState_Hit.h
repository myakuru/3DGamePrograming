#pragma once
#include"../PlayerState.h"
class PlayerState_Hit :public PlayerStateBase
{
public:
	PlayerState_Hit() = default;
	~PlayerState_Hit() override = default;

private:

	void StateStart() override;
	void StateUpdate() override;
	void StateEnd() override;

	//void ImGuiInspector() override;

	bool m_lButtonPressing = false;

};