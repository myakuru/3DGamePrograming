#pragma once
class StateBase : public KdGameObject
{
public:
	StateBase() = default;
	~StateBase() override = default;

	// ステートが開始されたときに呼び出される
	virtual void StateStart() = 0;

	// ステートが更新されるたびに呼び出される
	virtual void StateUpdate() = 0;

	// ステートが終了されたときに呼び出される
	virtual void StateEnd() = 0;
};