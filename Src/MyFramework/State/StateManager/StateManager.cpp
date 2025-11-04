#include "StateManager.h"
#include "../StateBase/StateBase.h"

void StateManager::ChangeState(std::shared_ptr<StateBase> _newState)
{
	// ステートが変わるときは前のステートを終了する
	if (m_nowState != nullptr)
	{
		m_nowState->StateEnd();
		m_nowState = nullptr;
	}

	// 新しいステートをセットする
	m_nowState = _newState;

	m_nowState->StateStart();
}

void StateManager::Update()
{
	if (m_nowState != nullptr)
	{
		m_nowState->StateUpdate();
	}
}