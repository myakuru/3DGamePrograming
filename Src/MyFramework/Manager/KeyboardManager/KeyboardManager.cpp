#include "KeyboardManager.h"
#include "Application/main.h"
#include "MyFramework/Manager/ImGuiManager/ImGuiManager.h"
#include "Application/Scene/SceneManager.h"

void KeyboardManager::Update(float deltaTime)
{
	// アプリウィンドウが非アクティブなら入力無視
	if (!IsAppWindowActive()) return;

	// ゲームシーンにマウスが入っていない場合は入力を無視
	if (!IMGUI_MANAGER.GetGameSceneInMouse()) return;

	if (SceneManager::Instance().IsIntroCamera()) return;

	for (int i = 0; i < 256; ++i)
	{
		mOldKeyState[i] = mNowKeyState[i];
		mNowKeyState[i] = GetAsyncKeyState(i) & 0x8000;

		if (IsKeyJustPressed(i))
		{
			mKeyPressDuration[i] = 0.0f;
		}
		else if (IsKeyPressed(i))
		{
			mKeyPressDuration[i] += deltaTime;
		}
		// IsKeyJustReleasedのフレームではリセットしない
		else if (!mNowKeyState[i] && !mOldKeyState[i])
		{
			mKeyPressDuration[i] = 0.0f;
		}
	}
}

bool KeyboardManager::IsAppWindowActive() const
{
	HWND hwnd = Application::Instance().GetWindowHandle();
	if (!hwnd) return false;

	// 最前面ウィンドウと一致するか
	if (const HWND& fg = GetForegroundWindow();
		fg != hwnd) return false;

	// 最小化されていたら無視
	if (IsIconic(hwnd)) return false;

	return true;
}
