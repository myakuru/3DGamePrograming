#pragma once

class XInputManager
{
public:
    // シングルトンインスタンスを取得（他のクラスでインスタンス作成できなくする）
    static XInputManager& GetInstance()
    {
        static XInputManager instance;
        return instance;
    }

    // XInputの状態を更新
    void Update()
    {
        ZeroMemory(&mState, sizeof(XINPUT_STATE));
        mResult = XInputGetState(0, &mState);
    }

    // Aボタンが押されているかチェック
    bool IsAPressed_A() const
    {
        return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_A);
    }
	// Bボタンが押されているかチェック
	bool IsBPressed_B() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_B);
	}
	// Xボタンが押されているかチェック
	bool IsXPressed_X() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_X);
	}
	// Yボタンが押されているかチェック
	bool IsYPressed_Y() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
	}
	// Startボタンが押されているかチェック
	bool IsStartPressed_Start() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_START);
	}
	// Backボタンが押されているかチェック
	bool IsBackPressed_Back() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
	}
	// Lスティックが押されているかチェック
	bool IsLThumbPressed_LThumb() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
	}
	// Rスティックが押されているかチェック
	bool IsRThumbPressed_RThumb() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
	}
	// Lショルダーが押されているかチェック
	bool IsLShoulderPressed_LShoulder() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
	}
	// Rショルダーが押されているかチェック
	bool IsRShoulderPressed_RShoulder() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
	}
	// 十字キー上が押されているかチェック
	bool IsDPadUpPressed_DPadUp() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
	}
	// 十字キー下が押されているかチェック
	bool IsDPadDownPressed_DPadDown() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
	}
	// 十字キー左が押されているかチェック
	bool IsDPadLeftPressed_DPadLeft() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
	}
	// 十字キー右が押されているかチェック
	bool IsDPadRightPressed_DPadRight() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
	}
	// Lトリガーが押されているかチェック
	bool IsLTriggerPressed_LTrigger() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	}
	// Rトリガーが押されているかチェック
	bool IsRTriggerPressed_RTrigger() const
	{
		return (mResult == ERROR_SUCCESS) && (mState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	}


    // コントローラが接続されているかチェック
    bool IsControllerConnected() const
    {
        return (mResult == ERROR_SUCCESS);
    }

    // 左スティックのX軸の値を取得（デッドゾーンを考慮しない）
    float GetLeftStickX() const
    {
        return (mResult == ERROR_SUCCESS) ? mState.Gamepad.sThumbLX / 32767.0f : 0.0f;
    }

    // 左スティックのY軸の値を取得（デッドゾーンを考慮しない）
    float GetLeftStickY() const
    {
        return (mResult == ERROR_SUCCESS) ? mState.Gamepad.sThumbLY / 32767.0f : 0.0f;
    }


private:
    // コンストラクタをプライベートにしてシングルトンにする
	XInputManager() : mState(), mResult(ERROR_DEVICE_NOT_CONNECTED), mDeadZone(0.2f) {}

    XINPUT_STATE mState;
    DWORD mResult;
    const float mDeadZone; // デッドゾーンの閾値
};
