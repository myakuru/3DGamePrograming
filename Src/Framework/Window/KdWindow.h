#pragma once

//=====================================================
//
// ウィンドウ
//
//=====================================================
class KdWindow {
public:

	//===================================
	// 取得・設定
	//===================================

	// ウィンドウハンドル取得
	HWND GetWndHandle() const { return m_hWnd; }
	// ウィンドウが存在する？
	bool IsCreated() const { return m_hWnd ? true : false; }
	// マウスホイールの変化量を取得
	int GetMouseWheelVal() const { return m_mouseWheelVal; }
	// インスタンスハンドル取得
	HINSTANCE GetInstanceHandle() const { return GetModuleHandle(0); }

	// クライアントサイズの設定
	void SetClientSize(int w, int h);

	//===================================
	// 初期化・解放
	//===================================

	// ウィンドウ作成
	bool Create(int clientWidth, int clientHeight, std::string_view titleName, std::string_view windowClassName);

	// 解放
	void Release();

	void SetBorderlessFullscreen(bool enable);
	bool IsBorderlessFullscreen() const { return m_borderless; }


	//
	~KdWindow() {
		Release();
	}

	//===================================
	// 処理
	//===================================

	// ウィンドウメッセージを処理する
	//  戻り値：終了メッセージが来たらfalseが返る
	bool ProcessMessage();

	// ファイルを開くダイアログボックスを表示
	// ・filepath		… 選択されたファイルパスが入る
	// ・title			… ウィンドウのタイトル文字
	// ・filters		… 指定された拡張子のみ表示されるようになる
	static bool OpenFileDialog(std::string& filepath, std::string_view title = "ファイルを開く", const char* filters = "全てのファイル\0*.*\0");

	// ファイル名をつけて保存ダイアログボックスを表示
	// ・filepath		… 選択されたファイルパスが入る
	// ・title			… ウィンドウのタイトル文字
	// ・filters		… 指定された拡張子のみ表示されるようになる
	// ・defExt			… ユーザーが拡張子を入力しなかった場合、これが自動的に付く
	static bool SaveFileDialog(std::string& filepath, std::string_view title = "ファイルを保存", const char* filters = "全てのファイル\0*.*\0", std::string_view defExt = "");

private:

	// ウィンドウハンドル
	HWND	m_hWnd = nullptr;

	// マウスホイール値
	int		m_mouseWheelVal = 0;

	// ウィンドウ関数
	static LRESULT CALLBACK callWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


	bool  m_borderless = false;			// 枠無しフルスクリーン中？
	DWORD m_savedStyle = 0;				// 元のウィンドウスタイル保存用
	RECT  m_savedRect = { 0,0,0,0 };	// 元のウィンドウ位置保存用

};
