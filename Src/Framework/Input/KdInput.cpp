#include "KdInput.h"


// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// InputManager
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 入力デバイス統合マネージャの更新
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 登録された入力デバイスのUpdateを行う
// 毎ループの初めに呼び出す必要がある
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputManager::Update()
{
	for (auto& device : m_pInputDevices)
	{
		device.second->Update();
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意のアプリケーションボタンの入力状態を取得
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 全ての有効状態の入力デバイスから任意のボタンの入力状状態を取得する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
short KdInputManager::GetButtonState(std::string_view name) const
{
	short buttonState = KdInputButtonBase::Free;

	for (auto& device : m_pInputDevices)
	{
		// 有効な時のみ入力に影響を与える
		if (device.second->GetActiveState() == KdInputCollector::ActiveState::Enable)
		{
			buttonState |= device.second->GetButtonState(name);
		}
	}

	return buttonState;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意のアプリケーションボタンが「押されていない状態」か判定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdInputManager::IsFree(std::string_view name) const
{
	return (GetButtonState(name) == KdInputButtonBase::Free);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意のアプリケーションボタンが「押された瞬間」か判定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdInputManager::IsPress(std::string_view name) const
{
	return (GetButtonState(name) & KdInputButtonBase::Press);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意のアプリケーションボタンが「押されている状態」か判定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdInputManager::IsHold(std::string_view name) const
{
	return (GetButtonState(name) & KdInputButtonBase::Hold);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意のアプリケーションボタンが「離された瞬間」か判定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdInputManager::IsRelease(std::string_view name) const
{
	return (GetButtonState(name) & KdInputButtonBase::Release);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意の軸入力状態を取得
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 指定した入力デバイスの任意の軸の入力状状態を2次元ベクトルで取得する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
Math::Vector2 KdInputManager::GetAxisState(std::string_view name) const
{
	float leftValue = 0.0f, rightValue = 0.0f, topValue = 0.0f, bottomValue = 0.0f;

	for (auto& collector : m_pInputDevices)
	{
		// 有効な時のみ入力に影響を与える
		if (collector.second->GetActiveState() == KdInputCollector::ActiveState::Enable)
		{
			Math::Vector2 nowDeviceAxis;

			nowDeviceAxis = collector.second->GetAxisState(name);

			// 入力が無ければスキップ
			if (nowDeviceAxis.LengthSquared() == 0.0f) { continue; }

			// 左右の入力をX軸数値で判定
			// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
			if (nowDeviceAxis.x < 0.0f)
			{
				// 左なら最小値を保持
				leftValue = std::min(nowDeviceAxis.x, leftValue);
			}
			else
			{
				// 右なら最大値を保持
				rightValue = std::max(nowDeviceAxis.x, rightValue);
			}

			// 上下の入力をY軸数値で判定
			// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
			if (nowDeviceAxis.y < 0.0f)
			{
				// 下なら最小値を保持
				bottomValue = std::min(nowDeviceAxis.y, bottomValue);
			}
			else
			{
				// 上なら最大値を保持
				topValue = std::max(nowDeviceAxis.y, topValue);
			}
		}
	}

	// 最終的に左右と上下の入力値をそれぞれ合成したものを出力する
	return Math::Vector2(leftValue + rightValue, topValue + bottomValue);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 入力コレクター（入力デバイス）の追加
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputManager::AddDevice(std::string_view name, KdInputCollector* pDevice)
{
	std::unique_ptr<KdInputCollector> pNewDevice(pDevice);

	AddDevice(name, pNewDevice);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputManager::AddDevice(std::string_view name, std::unique_ptr<KdInputCollector>& pDevice)
{
	m_pInputDevices[name.data()] = std::move(pDevice);
}

const std::unique_ptr<KdInputCollector>& KdInputManager::GetDevice(std::string_view name) const
{
	auto device = m_pInputDevices.find(name.data());

	if (device == m_pInputDevices.end())
	{
		assert(0 && "KdInputManager::GetDevice:未登録のデバイスです、名前を確認してください");
	}

	return device->second;
}

std::unique_ptr<KdInputCollector>& KdInputManager::WorkDevice(std::string_view name)
{
	auto device = m_pInputDevices.find(name.data());

	if (device == m_pInputDevices.end())
	{
		assert(0 && "KdInputManager::WorkDevice:未登録のデバイスです、名前を確認してください");
	}

	return device->second;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 登録している入力コレクターを全て解放
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputManager::Release()
{
	m_pInputDevices.clear();
}


// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// InputCollector
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 入力デバイスの入力状況更新
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 登録されたアプリケーションボタンに紐づけた入力キーを調査し、ボタンの入力状況を更新する
// 同じく軸情報の状況更新を行う
// 無効（入力を受け付けない）に設定している場合は全て入力無しにする
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputCollector::Update()
{
	// 更新前の準備　PreUpdate
	{
		for (auto& button : m_spButtons)
		{
			button.second->PreUpdate();
		}

		for (auto& axis : m_spAxes)
		{
			axis.second->PreUpdate();
		}
	}

	// 有効 or 監視：ボタンの状態を更新
	if (GetActiveState() != ActiveState::Disable)
	{
		for (auto& button : m_spButtons)
		{
			button.second->Update();
		}

		for (auto& axis : m_spAxes)
		{
			axis.second->Update();
		}
	}
	// 無効：全て入力されていない状態に更新
	else
	{
		for (auto& button : m_spButtons)
		{
			button.second->NoInput();
		}

		for (auto& axis : m_spAxes)
		{
			axis.second->NoInput();
		}
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 何かしらの入力を検知した時に「true」を返す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdInputCollector::IsSomethingInput()
{
	for (auto& button : m_spButtons)
	{
		if (!button.second) { continue; }
		
		// 入力を受け付けていたら「True」
		if (button.second->GetState() != KdInputButtonBase::State::Free ) { return true; }
	}

	for (auto& axis : m_spAxes)
	{
		if (!axis.second) { continue; }

		// 入力を受け付けていたら「True」
		if (axis.second->GetState().LengthSquared() != 0.0f) { return true; }
	}

	// 何の入力も検知できなかった
	return false;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意のアプリケーションボタンの入力情報取得
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
short KdInputCollector::GetButtonState(std::string_view name) const
{
	const std::shared_ptr<KdInputButtonBase>& spButton = GetButton(name);

	if (!spButton)
	{
		return KdInputButtonBase::Free;
	}

	return spButton->GetState();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 任意の軸の入力情報取得
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
Math::Vector2 KdInputCollector::GetAxisState(std::string_view name) const
{
	const std::shared_ptr<KdInputAxisBase>& spAxis = GetAxis(name);

	if (!spAxis)
	{
		return Math::Vector2::Zero;
	}

	return spAxis->GetState();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// アプリケーションボタンの追加
// 生ポインタの追加関数も用意（必ず new した生ポインタを引数として渡す事
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputCollector::AddButton(std::string_view name, KdInputButtonBase* pButton)
{
	AddButton(name.data(), std::shared_ptr<KdInputButtonBase>(pButton));
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputCollector::AddButton(std::string_view name, std::shared_ptr<KdInputButtonBase> spButton)
{
	m_spButtons[name.data()] = spButton;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputCollector::AddAxis(std::string_view name, KdInputAxisBase* pAxis)
{
	AddAxis(name.data(), std::shared_ptr<KdInputAxisBase>(pAxis));
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputCollector::AddAxis(std::string_view name, std::shared_ptr<KdInputAxisBase> spAxis)
{
	m_spAxes[name.data()] = spAxis;
}

const std::shared_ptr<KdInputButtonBase> KdInputCollector::GetButton(std::string_view name) const
{
	auto buttonIter = m_spButtons.find(name.data());

	if (buttonIter == m_spButtons.end())
	{
		assert(0 && "KdInputCollector::GetButton:指定されたボタンは未登録です、名前を確認してください");

		return nullptr;
	}

	return buttonIter->second;
}

const std::shared_ptr<KdInputAxisBase> KdInputCollector::GetAxis(std::string_view name) const
{
	auto AxisIter = m_spAxes.find(name.data());

	if (AxisIter == m_spAxes.end())
	{
		assert(0 && "KdInputCollector::GetAxis:指定された軸は未登録です、名前を確認してください");

		return nullptr;
	}

	return AxisIter->second;
}


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 管理しているボタンと入力軸・入力ポインタの解放
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputCollector::Release()
{
	m_spButtons.clear();
	m_spAxes.clear();
}


// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// InputButtonForWindows
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 引数付きコンストラクター：キーコードを指定してボタンを作成する
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 単一のキーコード（整数）
// 複数のキーコード（初期化リスト：整数）
// 複数のキーコード（Vector配列：整数）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
KdInputButtonForWindows::KdInputButtonForWindows(int keyCode)
{
	m_keyCodes.push_back(keyCode); 
}

KdInputButtonForWindows::KdInputButtonForWindows(std::initializer_list<int> keyCodeList)
{
	for (int keyCode : keyCodeList) m_keyCodes.push_back(keyCode);
}

KdInputButtonForWindows::KdInputButtonForWindows(const std::vector<int>& keyCodeList)
{
	for (int keyCode : keyCodeList) m_keyCodes.push_back(keyCode);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// GetAsyncKeyStateを使ってアプリケーションボタンに紐づけたキーコードの入力状況を取得
// その情報を基にアプリケーションボタンの入力状況を更新する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputButtonForWindows::Update()
{
	// 既に状態更新済み
	if (!m_needUpdate) { return; }

	short keyState = 0;

	for (int keyCode : m_keyCodes)
	{
		keyState |= GetAsyncKeyState(keyCode);
	}

	if (keyState & 0x8000)
	{
		if (m_state & Hold)
		{
			m_state &= ~Press;
		}
		else
		{
			m_state |= Press | Hold;
		}
	}
	else
	{
		if (m_state & Hold)
		{
			m_state &= ~Press;
			m_state &= ~Hold;
			m_state |= Release;
		}
		else
		{
			m_state &= ~Release;
		}
	}

	m_needUpdate = false;
}

void KdInputButtonForWindows::GetCode(std::vector<int>& ret) const
{
	// 登録された全ての入力コードを受け取る
	for (int code : m_keyCodes)
	{
		ret.push_back(code);
	}
}



// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// InputAxisBase
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 軸の入力状況の取得
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 軸の数値に倍率を合成
// 限界値に設定された値を超えないようにclamp
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
Math::Vector2 KdInputAxisBase::GetState() const
{
	Math::Vector2 retAxis = m_axis * m_valueRate;

	retAxis.x = std::clamp(retAxis.x, -m_limitValue, m_limitValue);
	retAxis.y = std::clamp(retAxis.y, -m_limitValue, m_limitValue);

	return retAxis;
}


// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// InputAxisForWindowsKeyBoard
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 引数付きコンストラクター：キーコードを指定して上下左右ボタンを作成・登録する
// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// 上下左右4ボタンに1つのキーコード（整数）
// 上下左右4ボタンに複数のキーコード（初期化リスト：整数）
// 上下左右4ボタンに複数のキーコード（Vector配列：整数）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
KdInputAxisForWindows::KdInputAxisForWindows(int upCode, int rightCode, int downCode, int leftCode)
{
	m_spDirButtons.resize(DIR::Max);

	m_spDirButtons[Up] = std::make_shared<KdInputButtonForWindows>(upCode);
	m_spDirButtons[Right] = std::make_shared<KdInputButtonForWindows>(rightCode);
	m_spDirButtons[Down] = std::make_shared<KdInputButtonForWindows>(downCode);
	m_spDirButtons[Left] = std::make_shared<KdInputButtonForWindows>(leftCode);
}

KdInputAxisForWindows::KdInputAxisForWindows(std::initializer_list<int> upCodes,
	std::initializer_list<int> rightCodes, std::initializer_list<int> downCodes, std::initializer_list<int> leftCodes)
{
	m_spDirButtons.resize(DIR::Max);

	m_spDirButtons[Up] = std::make_shared<KdInputButtonForWindows>(upCodes);
	m_spDirButtons[Right] = std::make_shared<KdInputButtonForWindows>(rightCodes);
	m_spDirButtons[Down] = std::make_shared<KdInputButtonForWindows>(downCodes);
	m_spDirButtons[Left] = std::make_shared<KdInputButtonForWindows>(leftCodes);
}

KdInputAxisForWindows::KdInputAxisForWindows(const std::vector<int>& upCodes, const std::vector<int>& rightCodes,
	const std::vector<int>& downCodes, const std::vector<int>& leftCodes)
{
	m_spDirButtons.resize(DIR::Max);

	m_spDirButtons[Up] = std::make_shared<KdInputButtonForWindows>(upCodes);
	m_spDirButtons[Right] = std::make_shared<KdInputButtonForWindows>(rightCodes);
	m_spDirButtons[Down] = std::make_shared<KdInputButtonForWindows>(downCodes);
	m_spDirButtons[Left] = std::make_shared<KdInputButtonForWindows>(leftCodes);
}

KdInputAxisForWindows::KdInputAxisForWindows(const std::shared_ptr<KdInputButtonBase> upButton,
	const std::shared_ptr<KdInputButtonBase> rightButton,
	const std::shared_ptr<KdInputButtonBase> downButton, 
	const std::shared_ptr<KdInputButtonBase> leftButton)
{
	m_spDirButtons.push_back(upButton);
	m_spDirButtons.push_back(rightButton);
	m_spDirButtons.push_back(downButton);
	m_spDirButtons.push_back(leftButton);
}

void KdInputAxisForWindows::PreUpdate()
{
	for (auto& button : m_spDirButtons)
	{
		button->PreUpdate();
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// GetAsyncKeyStateを使って軸に紐づけた上下左右のキーコードの入力状況を更新
// その情報を基に軸の入力状況を更新する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputAxisForWindows::Update()
{
	m_axis = Math::Vector2::Zero;

	for (auto& dirButton : m_spDirButtons)
	{
		dirButton->Update();
	}

	if (m_spDirButtons[Up]->GetState())		{ m_axis.y += 1.0f; }
	if (m_spDirButtons[Right]->GetState())	{ m_axis.x += 1.0f; }
	if (m_spDirButtons[Down]->GetState())	{ m_axis.y -= 1.0f; }
	if (m_spDirButtons[Left]->GetState())	{ m_axis.x -= 1.0f; }
}


// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// InputAxisForWindowsMouse
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####

KdInputAxisForWindowsMouse::KdInputAxisForWindowsMouse(int fixCode)
{
	m_spFixButton = std::make_shared<KdInputButtonForWindows>(fixCode);
}

KdInputAxisForWindowsMouse::KdInputAxisForWindowsMouse(std::initializer_list<int> fixCodes)
{
	m_spFixButton = std::make_shared<KdInputButtonForWindows>(fixCodes);
}

KdInputAxisForWindowsMouse::KdInputAxisForWindowsMouse(const std::vector<int>& fixCodes)
{
	m_spFixButton = std::make_shared<KdInputButtonForWindows>(fixCodes);
}

KdInputAxisForWindowsMouse::KdInputAxisForWindowsMouse(const std::shared_ptr<KdInputButtonBase> fixButton)
{
	m_spFixButton = fixButton;
}

void KdInputAxisForWindowsMouse::PreUpdate()
{
	if (!m_spFixButton) { return; }

	m_spFixButton->PreUpdate();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// マウスの毎フレームの移動量を使って軸の入力状況を更新
// 初めの1フレーム目はマウスの前フレーム座標が無く移動量を求められないので0ベクトル
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdInputAxisForWindowsMouse::Update()
{
	bool needCreateAxisState = true;
	bool needUpdatePrevPos = true;

	// 軸固定モードで固定ボタンが押されている時は軸情報を作成し、軸の中心を更新しない
	if (m_spFixButton)
	{
		m_spFixButton->Update();

		if (m_spFixButton->GetState())
		{
			needUpdatePrevPos = false;
		}
		else
		{
			needCreateAxisState = false;
		}
	}

	POINT nowPos;

	GetCursorPos(&nowPos);

	// 開始フレームではない ＋ 軸情報の生成を必要とする時
	if (!m_beginFrame && needCreateAxisState)
	{
		m_axis.x = float(nowPos.x - m_prevMousePos.x);
		m_axis.y = float(m_prevMousePos.y - nowPos.y);
	}
	else
	{
		m_axis = Math::Vector2::Zero;
	}

	if (needUpdatePrevPos)
	{
		// 座標を保持、次回以降はこの座標との差で移動量を求める
		m_prevMousePos = nowPos;
	}

	m_beginFrame = false;
}
