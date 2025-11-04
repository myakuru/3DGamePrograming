#pragma once


class KdInputCollector;

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 様々な入力を管理するクラス：複数のInputCollectorを管理
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// キーボードやゲームパッド等の複数の入力を同時に受付可能（各デバイスの有効無効の切り替え機能あり）
// 運用はUpdate()を毎ループの初めに呼び出す必要がある
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdInputManager
{
public:

	static KdInputManager& Instance()
	{
		static KdInputManager instance;
		return instance;
	}

	void Update();

	// 全ての有効な入力装置からのボタン入力状態を取得
	short GetButtonState(std::string_view name) const;

	bool IsFree(std::string_view name) const;
	bool IsPress(std::string_view name) const;
	bool IsHold(std::string_view name) const;
	bool IsRelease(std::string_view name) const;

	// 全ての有効な入力装置からの軸入力状態を取得
	Math::Vector2 GetAxisState(std::string_view name) const;

	// 入力装置の登録
	void AddDevice(std::string_view name, KdInputCollector* pDevice);
	void AddDevice(std::string_view name, std::unique_ptr<KdInputCollector>& pDevice);

	const std::unique_ptr<KdInputCollector>& GetDevice(std::string_view name) const;
	std::unique_ptr<KdInputCollector>& WorkDevice(std::string_view name);

	void Release();

private:

	KdInputManager() {}
	~KdInputManager() { Release(); }

	std::unordered_map<std::string, std::unique_ptr<KdInputCollector>> m_pInputDevices;
};


class KdInputButtonBase;
class KdInputAxisBase;

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 単一の入力デバイスからの入力をコレクションするクラス
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// キーボードやゲームパッド等、それぞれのInputCollecterが必要
// ゲームで使う入力Indexの管理もここで行う
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdInputCollector
{
public:

	enum class ActiveState
	{
		Disable,	// 無効：完全に停止している状態
		Monitoring,	// 監視：デバイスの入力を更新、アプリに入力の影響はない
		Enable,		// 有効：アプリに入力の影響を与える
	};

	KdInputCollector() {}
	~KdInputCollector() {}

	void Update();

	// 何かしらの入力を検知したか
	bool IsSomethingInput();

	// 任意の入力状況の取得
	short GetButtonState(std::string_view name) const;
	Math::Vector2 GetAxisState(std::string_view name) const;

	// 入力デバイスの状態の取得と設定
	ActiveState GetActiveState() const { return m_state; }
	void SetActiveState(ActiveState state) { m_state = state; }

	// アプリケーションボタンの追加：上書き
	void AddButton(std::string_view name, KdInputButtonBase* pButton);
	void AddButton(std::string_view name, std::shared_ptr<KdInputButtonBase> spButton);
	// 入力軸の追加：上書き
	void AddAxis(std::string_view name, KdInputAxisBase* pAxis);
	void AddAxis(std::string_view name, std::shared_ptr<KdInputAxisBase> spAxis);

	const std::shared_ptr<KdInputButtonBase> GetButton(std::string_view name) const;

	const std::shared_ptr<KdInputAxisBase> GetAxis(std::string_view name) const;

private:

	void Release();

	std::unordered_map<std::string, std::shared_ptr<KdInputButtonBase>> m_spButtons;

	std::unordered_map<std::string, std::shared_ptr<KdInputAxisBase>> m_spAxes;

	ActiveState m_state = ActiveState::Enable;
};


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 単一のボタンの入力状態を保持する機能を持った基底クラス
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// ボタンの入力状態を示すBitFlag（enum State）
// BitFlagの組み合わせでボタンの入力状態をパラメータとして保持する機能を持つ
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdInputButtonBase
{
public:

	enum State
	{
		Free,				// 入力が無い
		Press,				// 押された瞬間
		Hold = Press << 1,	// 押している間
		Release = Press << 2// 離された瞬間
	};

	KdInputButtonBase() {}
	virtual ~KdInputButtonBase() {}

	void PreUpdate() { m_needUpdate = true; }

	virtual void Update() = 0;

	// 強制的に入力無しの状態にする
	void NoInput() { m_state = Free; }

	short GetState() const { return m_state; }

	virtual void GetCode(std::vector<int>& ret) const = 0;

protected:
	// 入力の状態
	short m_state = Free;

	// 重複しての更新を防ぐ
	bool m_needUpdate = true;
};


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// WinAPIのGetAsyncKeyStateを利用したキー制御：マウスとキーボードの入力
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 複数の入力キーコードを登録できる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdInputButtonForWindows : public KdInputButtonBase
{
public:

	// 引き付きコンストラクター
	KdInputButtonForWindows(int keyCode);
	KdInputButtonForWindows(std::initializer_list<int> keyCodeList);
	KdInputButtonForWindows(const std::vector<int>& keyCodeList);

	void Update() override;

	void GetCode(std::vector<int>& ret) const override;

private:
	std::list<int>   m_keyCodes;
};


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 単一の軸の入力状態を保持する機能を持った基底クラス
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// ジョイスティックや十字キーなど入力を二次元の移動ベクトルに変換して保持する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdInputAxisBase
{
public:
	KdInputAxisBase(){}
	virtual ~KdInputAxisBase(){}

	virtual void PreUpdate() {}

	virtual void Update() = 0;

	// 強制的に入力無しの状態にする
	void NoInput() { m_axis = Math::Vector2::Zero; }

	void SetValueRate(float rate) { m_valueRate = rate; }
	void SetLimitValue(float limit) { m_limitValue = limit; }

	Math::Vector2 GetState() const;

protected:
	Math::Vector2	m_axis;

	// 軸の数値にかける補正
	float m_valueRate = 1.0f;

	// 軸の限界値
	float m_limitValue = FLT_MAX;
};


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// WinAPIのGetAsyncKeyStateの入力を利用した軸制御
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 指定した上下左右のキーの入力状況を軸情報として保持する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdInputAxisForWindows : public KdInputAxisBase
{
public:

	// 引き付きコンストラクター
	KdInputAxisForWindows(int upCode, int rightCode, int downCode, int leftCode);
	KdInputAxisForWindows(std::initializer_list<int> upCodes, std::initializer_list<int> rightCodes,
		std::initializer_list<int> downCodes, std::initializer_list<int> leftCodes);
	KdInputAxisForWindows(const std::vector<int>& upCodes, const std::vector<int>& rightCodes,
		const std::vector<int>& downCodes, const std::vector<int>& leftCodes);
	KdInputAxisForWindows(const std::shared_ptr<KdInputButtonBase> upButton, 
		const std::shared_ptr<KdInputButtonBase> rightButton, 
		const std::shared_ptr<KdInputButtonBase> downButton, 
		const std::shared_ptr<KdInputButtonBase> leftButton);

	void PreUpdate() override;

	void Update() override;

private:

	enum DIR
	{
		Up, Right, Down, Left, Max
	};

	std::vector<std::shared_ptr<KdInputButtonBase>> m_spDirButtons;
};


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// Windowsのマウス移動を利用した軸制御
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// マウスの移動量を軸情報として保持する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdInputAxisForWindowsMouse : public KdInputAxisBase
{
public:

	KdInputAxisForWindowsMouse() {}

	KdInputAxisForWindowsMouse(int fixCode);
	KdInputAxisForWindowsMouse(std::initializer_list<int> fixCodes);
	KdInputAxisForWindowsMouse(const std::vector<int>& fixCodes);
	KdInputAxisForWindowsMouse(const std::shared_ptr<KdInputButtonBase> fixButton);

	void PreUpdate() override;

	void Update() override;

private:
	POINT m_prevMousePos = { 0 ,0 };

	bool m_beginFrame = true;

	// マウスの1フレームの移動量ではなく、固定された中心位置からの現座標を軸情報として管理する
	// マウスで疑似ジョイスティック操作をしたり、スマホの疑似コントローラーのような動作をさせたい時に利用。
	// 押している間、軸の中心位置が固定される
	std::shared_ptr<KdInputButtonBase>	m_spFixButton;
};
