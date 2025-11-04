#pragma once

class KeyboardManager
{
public:
	// シングルトンインスタンス取得
	static KeyboardManager& GetInstance()
	{
		static KeyboardManager instance;
		return instance;
	}

	// mainループで毎フレーム呼び出す
	void Update(float deltaTime);

	bool IsKeyPressed(int key) const
	{
		return mNowKeyState[key];
	}

	bool IsKeyJustPressed(int key) const
	{
		return mNowKeyState[key] && !mOldKeyState[key];
	}

	bool IsKeyJustReleased(int key) const
	{
		return !mNowKeyState[key] && mOldKeyState[key];
	}

	// 押した時間を取得
	float GetKeyPressDuration(int key) const
	{
		return mKeyPressDuration[key];
	}

private:
	KeyboardManager() = default;

	std::array<SHORT, 256> mNowKeyState;
	std::array<SHORT, 256> mOldKeyState;
	std::array<float, 256> mKeyPressDuration;
};