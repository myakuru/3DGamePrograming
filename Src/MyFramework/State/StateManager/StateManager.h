#pragma once
#include "../../State/StateBase/StateBase.h"

template<typename OwnerType>
class StateManager
{
public:
	StateManager() : m_fnChangeState([]() {}) {}
	~StateManager() = default;

	// 既存インスタンスで状態変更
	void ChangeState(std::shared_ptr<StateBase<OwnerType>> newState)
	{
		m_fnChangeState = [this, newState = std::move(newState)]()
			{
				if (m_pOwner == nullptr) return;
				if (!newState) return;

				// 旧ステートを保持
				std::shared_ptr<StateBase<OwnerType>> prev = m_spNowState;

				// 旧ステート終了
				if (prev)
				{
					prev->CallExit(m_pOwner);
				}

				// 新ステートへ差し替え
				m_spNowState = newState;
				m_spNowState->SetMachine(this);

				// プロトタイプ適用（旧→新）
				if (prev)
				{
					(void)m_spNowState->TryApplyFromPrototype(*prev);
				}

				// 開始
				m_spNowState->CallStart(m_pOwner);
			};
	}

	// ステートマシン開始（オーナー設定）
	void Start(OwnerType* a_pOwner)
	{
		m_pOwner = a_pOwner;
		m_fnChangeState = []() {};
	}

	// コンストラクタ引数から新しいステートを生成して切り替え
	template<typename StateType, typename... ArgType>
	void ChangeState(ArgType&&... a_args)
	{
		using TupleT = std::tuple<std::decay_t<ArgType>...>;
		auto argsPtr = std::make_shared<TupleT>(std::forward<ArgType>(a_args)...);

		m_fnChangeState = [this, argsPtr]() mutable
			{
				if (m_pOwner == nullptr) return;

				// 旧ステートを保持
				std::shared_ptr<StateBase<OwnerType>> prev = m_spNowState;

				// 旧ステート終了
				if (prev) { prev->CallExit(m_pOwner); }

				// 新規生成（コピー可能なキャプチャのみ）
				auto newState = std::apply(
					[](auto&&... xs)
					{
						return std::make_shared<StateType>(std::forward<decltype(xs)>(xs)...);
					},
					*argsPtr);

				if (!newState) return;

				// 差し替え & マシン設定
				m_spNowState = std::move(newState);
				m_spNowState->SetMachine(this);

				// プロトタイプ適用（旧→新）
				if (prev) { (void)m_spNowState->TryApplyFromPrototype(*prev); }

				// 開始
				m_spNowState->CallStart(m_pOwner);
			};
	}

	// 状態更新
	void Update()
	{
		// ステートの変更命令があれば処理する
		m_fnChangeState();
		m_fnChangeState = []() {};

		if (m_spNowState)
		{
			m_spNowState->CallUpdate(m_pOwner);
		}
	}

	// 現在のステートを取得
	std::shared_ptr<StateBase<OwnerType>> GetCurrentState() const { return m_spNowState; }

private:
	// 状態の持ち主
	OwnerType* m_pOwner = nullptr;

	// 今のステート
	std::shared_ptr<StateBase<OwnerType>> m_spNowState = nullptr;

	// ステートの変更命令を保存しておく関数オブジェクト
	std::function<void()> m_fnChangeState;
};