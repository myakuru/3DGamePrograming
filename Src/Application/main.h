#pragma once
class JsonManager;
class ImGuiManager;
//============================================================
// アプリケーションクラス
//	APP.～ でどこからでもアクセス可能
//============================================================
class Application
{
// メンバ
public:

	// アプリケーション実行
	void Execute();

	// アプリケーション終了
	void End()							{ m_endFlag = true; }

	HWND GetWindowHandle()		const	{ return m_window.GetWndHandle(); }
	int GetMouseWheelValue()	const	{ return m_window.GetMouseWheelVal(); }

	KdWindow& GetWindow()				{ return m_window; }

	int GetNowFPS()				const	{ return m_fpsController.m_nowfps; }
	int GetMaxFPS()				const	{ return m_fpsController.m_maxFps; }

	// デルタタイムのゲッター
	float GetDeltaTime() const { return m_fpsController.m_deltaTime; }
	float GetUnscaledDeltaTime() const { return m_fpsController.m_unscaledDeltaTime; }


	float SetFpsScale(float _fpsScale)
	{
		m_fpsController.m_timeScale = _fpsScale;
		return m_fpsController.m_timeScale;
	}

	void ResizeWindow(int w, int h);

private:

	void KdBeginUpdate();
	void PreUpdate();
	void Update();
	void PostUpdate();
	void KdPostUpdate();

	void KdBeginDraw(bool usePostProcess = true);
	void PreDraw();
	void Draw();
	void PostDraw();
	void DrawSprite();
	void KdPostDraw();

	// アプリケーション初期化
	bool Init(int w, int h);

	// アプリケーション解放
	void Release();

	// ゲームウィンドウクラス
	KdWindow		m_window;

	// FPSコントローラー
	KdFPSController	m_fpsController;

	// ゲーム終了フラグ trueで終了する
	bool		m_endFlag = false;

//=====================================================
// シングルトンパターン
//=====================================================
private:
	// 
	Application() {}

	std::shared_ptr<JsonManager> m_jsonManager;
	std::shared_ptr<ImGuiManager> m_imguiManager;

public:
	static Application &Instance(){
		static Application Instance;
		return Instance;
	}

	std::shared_ptr<JsonManager> GetJsonManager() { return m_jsonManager; }
	std::shared_ptr<ImGuiManager> GetImGuiManager() { return m_imguiManager; }

};

#define JSON_MANAGER (*Application::Instance().GetJsonManager())
#define IMGUI_MANAGER (*Application::Instance().GetImGuiManager())
