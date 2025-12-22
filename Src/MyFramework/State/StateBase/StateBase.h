#pragma once
template<typename OwnerType>
class StateManager;

// 全ての状態の基底となるクラス
template<typename OwnerType>
class StateBase
{
protected:

	friend class StateManager<OwnerType>;

public:
	StateBase() = default;
	virtual ~StateBase() = default;

	// ステートが開始されたときに呼び出される
	virtual void StateStart(OwnerType* _pOwner) = 0;

	// ステートが更新されるたびに呼び出される
	virtual void StateUpdate(OwnerType* _pOwner) = 0;

	// ステートが終了されたときに呼び出される
	virtual void StateEnd(OwnerType* _pOwner) = 0;

	// プロトタイプ適用のためのフック（各派生がオーバーライド）
	virtual bool TryApplyFromPrototype(const StateBase<OwnerType>&) { return false; }

	// パラメータ編集
	virtual void ExposeParametersImGui() {}
	// JSON 読み込み　/ 書き出し
	virtual void JsonInput(const nlohmann::json& _json) { (void)_json; }
	virtual void JsonSave(nlohmann::json& _json) const { (void)_json; }

	StateManager<OwnerType>* GetStateMachine() { return m_pMachine; }

private:

	// この状態を管理しているステートマシーンをセット
	void SetMachine(StateManager<OwnerType>* a_pMachine)
	{
		m_pMachine = a_pMachine;
	}

	// 開始関数をマシンから呼ぶための関数
	void CallStart(OwnerType* _pOwner) // 仮想関数をマシンが安全に呼ぶための関数を追加
	{
		if (m_pMachine == nullptr || _pOwner == nullptr)
		{
			return;
		}
		StateStart(_pOwner);
	}

	// 更新関数をマシンから呼ぶための関数
	void CallUpdate(OwnerType* _pOwner)
	{
		if (m_pMachine == nullptr || _pOwner == nullptr)
		{
			return;
		}
		StateUpdate(_pOwner);
	}

	// 終了関数をマシンから呼ぶための関数
	void CallExit(OwnerType* _pOwner)
	{
		if (m_pMachine == nullptr || _pOwner == nullptr)
		{
			return;
		}
		StateEnd(_pOwner);
	}

protected:

	StateManager<OwnerType>* m_pMachine = nullptr; // このステートを管理しているステートマシンのポインタを保存
};