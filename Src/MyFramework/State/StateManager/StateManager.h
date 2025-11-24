#pragma once
class StateBase;
class StateManager
{
public:
	StateManager();
	~StateManager();

	// ステートの変更
	void ChangeState(std::shared_ptr<StateBase> _newState);

	// ステートの更新
	void Update();

	// 現在のステートを取得
	std::shared_ptr<StateBase> GetCurrentState() const { return m_nowState; }

private:
	std::shared_ptr<StateBase> m_nowState = nullptr;

};