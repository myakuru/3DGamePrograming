#pragma once
class StateBase;
class StateManager
{
public:
	StateManager() = default;
	~StateManager() = default;

	// ステートの変更
	void ChangeState(std::shared_ptr<StateBase> _newState);

	// ステートの更新
	void Update();

private:
	std::shared_ptr<StateBase> m_nowState = nullptr;

};