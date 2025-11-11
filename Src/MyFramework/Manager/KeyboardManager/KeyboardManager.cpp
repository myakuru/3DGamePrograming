#include "KeyboardManager.h"
#include "Application/main.h"
#include "MyFramework/Manager/ImGuiManager/ImGuiManager.h"

void KeyboardManager::Update(float deltaTime)
{
	// ゲームシーンにマウスが入っていない場合は入力を無視
	if (!IMGUI_MANAGER.GetGameSceneInMouse()) return;

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
