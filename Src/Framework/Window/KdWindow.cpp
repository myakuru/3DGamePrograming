#include "Framework/KdFramework.h"

#include "KdWindow.h"

// ImGui
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool KdWindow::Create(int clientWidth, int clientHeight, std::string_view titleName, std::string_view windowClassName)
{
	// インスタンスハンドル取得
	HINSTANCE hInst = GetInstanceHandle();

	//===================================================================
	// メインウィンドウ作成
	//===================================================================

	std::wstring wndClsName = sjis_to_wide(windowClassName.data());

	//ウィンドウクラスの定義
	WNDCLASSEX wc;											// ウィンドウクラスの定義用
	wc.cbSize = sizeof(WNDCLASSEX);							// 構造体のサイズ
	wc.style = 0;											// スタイル
	wc.lpfnWndProc = &KdWindow::callWindowProc;				// ウインドウ関数
	wc.cbClsExtra = 0;										// エキストラクラス情報 
	wc.cbWndExtra = 0;										// エキストラウィンドウ情報
	wc.hInstance = hInst;									// インスタンスハンドル
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);			// ラージアイコン
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);		// スモールアイコン 
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);			// マウスカーソル
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// 背景色
	wc.lpszMenuName = nullptr;								// メインメニュー名
	wc.lpszClassName = wndClsName.c_str();					// ウィンドウクラス名

	//ウィンドウクラスの登録
	if (!RegisterClassEx(&wc)) {
		return false;
	}

	//ウィンドウの作成
	m_hWnd = CreateWindow(
		wndClsName.c_str(),									// ウィンドウクラス名
		sjis_to_wide(titleName.data()).c_str(),					// ウィンドウのタイトル
		WS_OVERLAPPEDWINDOW - WS_THICKFRAME - WS_MAXIMIZEBOX,				// ウィンドウタイプを標準タイプに	
		0,													// ウィンドウの位置（Ｘ座標）
		0,													// ウィンドウの位置（Ｙ座標）						
		clientWidth,										// ウィンドウの幅
		clientHeight,										// ウィンドウの高さ			
		nullptr,											// 親ウィンドウのハンドル
		nullptr,											// メニューのハンドル
		hInst,												// インスタンスハンドル 
		this);												// 追加情報

	if (m_hWnd == nullptr) {
		return false;
	}

	// クライアントのサイズを設定
	SetClientSize(clientWidth, clientHeight);

	//===================================================================
	//ウィンドウの表示
	//===================================================================
	ShowWindow(m_hWnd, SW_SHOW);
	//ウィンドウの更新
	UpdateWindow(m_hWnd);

	// timeGetTime関数の精度を1msに設定
	timeBeginPeriod(1);

	return true;
}

void KdWindow::Release()
{
	if (m_hWnd) {
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}
}

void KdWindow::SetBorderlessFullscreen(bool enable)
{
	if (!m_hWnd) return;

	if (enable && !m_borderless) 
	{
		// スタイル・位置保存
		m_savedStyle = GetWindowLong(m_hWnd, GWL_STYLE);

		// Windowの矩形情報を取得
		GetWindowRect(m_hWnd, &m_savedRect);

		// モニタ全面へ
		HMONITOR mon = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

		// モニタ情報構造体
		MONITORINFO mi{ sizeof(mi) };

		// モニタ情報を取得
		GetMonitorInfo(mon, &mi);

		// Windowのスタイルを変更し、位置・サイズをモニタ全面に
		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

		// ウィンドウ位置・サイズを変更　：モニターの位置・サイズに合わせてフルスクリーンにするイメージ
		SetWindowPos(m_hWnd, HWND_TOP,
			mi.rcMonitor.left, mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER);

		// ウィンドウを再表示
		ShowWindow(m_hWnd, SW_SHOW);

		m_borderless = true;
	}
	else if (!enable && m_borderless)
	{
		// 以前のウィンドウスタイル・位置に戻す
		SetWindowLong(m_hWnd, GWL_STYLE, m_savedStyle);
		SetWindowPos(m_hWnd, nullptr,
			m_savedRect.left, m_savedRect.top,
			m_savedRect.right - m_savedRect.left,
			m_savedRect.bottom - m_savedRect.top,
			SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
		ShowWindow(m_hWnd, SW_SHOW);

		m_borderless = false;
	}
}

bool KdWindow::ProcessMessage()
{
	m_mouseWheelVal = 0;

	// メッセージ取得
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// 終了メッセージがきた
		if (msg.message == WM_QUIT) {
			return false;
		}

		//メッセージ処理
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

// ウィンドウ関数(Static関数)
LRESULT CALLBACK KdWindow::callWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ウィンドウプロパティから、GameWindowクラスのインスタンスを取得
	KdWindow* pThis = (KdWindow*)GetProp(hWnd, L"GameWindowInstance");

	// nullptrの場合は、デフォルト処理を実行
	if (pThis == nullptr) {
		switch (message) {
			case WM_CREATE:
			{
				// CreateWindow()で渡したパラメータを取得
				CREATESTRUCT * createStruct = (CREATESTRUCT*)lParam;
				KdWindow* window = (KdWindow*)createStruct->lpCreateParams;

				// ウィンドウプロパティにこのクラスのインスタンスアドレスを埋め込んでおく
				// 次回から、pThis->WindowProcの方へ処理が流れていく
				SetProp(hWnd, L"GameWindowInstance", window);

			}
			return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	// インスタンス側のWindow関数を実行する
	return pThis->WindowProc(hWnd, message, wParam, lParam);
}

// ウィンドウ関数
LRESULT KdWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ImGuiにイベント通知
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
		return true;
	}

	//===================================
	//メッセージによって処理を選択
	//===================================
	switch (message) {
	// ホイールスクロール時
	case WM_MOUSEWHEEL:
		{
			m_mouseWheelVal = (short)HIWORD(wParam);
		}
		break;
	// Xボタンが押された
	case WM_CLOSE:
		// 破棄
		Release();
		break;
	// ウィンドウ破棄直前
	case WM_DESTROY:
		RemoveProp(hWnd, L"GameWindowInstance");
		PostQuitMessage(0);
		break;
	case WM_SYSKEYDOWN:
		// ALT+Enter検出（lParamのbit29がALT）
		if (wParam == VK_RETURN && (lParam & (1 << 29))) {
			SetBorderlessFullscreen(!IsBorderlessFullscreen());
			return 0;
		}
		break;

	case WM_MENUCHAR:
		// メニューが無いときのALT系入力でビープになるのを抑止
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) {
			UINT newW = LOWORD(lParam);
			UINT newH = HIWORD(lParam);
			if (newW && newH) {
				// バックバッファのみリサイズ
				KdDirect3D::Instance().Resize((int)newW, (int)newH);
			}
		}
		break;
	default:
		// メッセージのデフォルト処理
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ウィンドウのクライアントのサイズを指定サイズにする。
void KdWindow::SetClientSize(int w, int h)
{
	RECT rcWnd, rcCli;

	GetWindowRect(m_hWnd, &rcWnd); // ウィンドウのRECT取得
	GetClientRect(m_hWnd, &rcCli); // クライアント領域のRECT取得

	// ウィンドウの余白を考えて、クライアントのサイズを指定サイズにする。
	MoveWindow(m_hWnd,
		rcWnd.left,// X座標
		rcWnd.top,// Y座標
		w + (rcWnd.right - rcWnd.left) - (rcCli.right - rcCli.left),
		h + (rcWnd.bottom - rcWnd.top) - (rcCli.bottom - rcCli.top),
		TRUE);
}

bool KdWindow::OpenFileDialog(std::string& filepath, std::string_view title, const char* filters)
{
	// 現在のカレントディレクトリ保存
	auto current = std::filesystem::current_path();
	// ファイル名のみ
	auto filename = std::filesystem::path(filepath).filename();

	// 結果用
	static char fname[1000];
	strcpy_s(fname, sizeof(fname), filename.string().c_str());

	// デフォルトフォルダ
	std::string dir;
	if (filepath.size() == 0)
	{
		dir = current.string() + "\\";
	}
	else {
		auto path = std::filesystem::absolute(filepath);
		dir = path.parent_path().string() + "\\";
	}

	OPENFILENAMEA o;
	ZeroMemory(&o, sizeof(o));

	o.lStructSize = sizeof(o);									// 構造体サイズ
	o.hwndOwner = nullptr;										// 親ウィンドウのハンドル
	o.lpstrInitialDir = dir.c_str();							// 初期フォルダー
	o.lpstrFile = fname;										// 取得したファイル名を保存するバッファ
	o.nMaxFile = sizeof(fname);									// 取得したファイル名を保存するバッファサイズ
	o.lpstrFilter = filters;									// (例) "TXTファイル(*.TXT)\0*.TXT\0全てのファイル(*.*)\0*.*\0";
	o.lpstrDefExt = "";
	o.lpstrTitle = title.data();
	o.nFilterIndex = 1;
	if (GetOpenFileNameA(&o))
	{
		// カレントディレクトリを元に戻す
		std::filesystem::current_path(current);
		// 相対パスへ変換
		filepath = std::filesystem::relative(fname).string();
		return true;
	}
	std::filesystem::current_path(current);	// カレントディレクトリを元に戻す
	return false;
}

bool KdWindow::SaveFileDialog(std::string& filepath, std::string_view title, const char* filters, std::string_view defExt)
{
	// 現在のカレントディレクトリ保存
	auto current = std::filesystem::current_path();
	// ファイル名のみ
	auto filename = std::filesystem::path(filepath).filename();

	// 結果用
	static char fname[1000];
	strcpy_s(fname, sizeof(fname), filename.string().c_str());

	// デフォルトフォルダ
	std::string dir;
	if (filepath.size() == 0)
	{
		dir = current.string() + "\\";
	}
	else {
		auto path = std::filesystem::absolute(filepath);
		dir = path.parent_path().string() + "\\";
	}

	OPENFILENAMEA o;
	ZeroMemory(&o, sizeof(o));

	o.lStructSize = sizeof(o);									// 構造体サイズ
	o.hwndOwner = nullptr;										// 親ウィンドウのハンドル
	o.lpstrInitialDir = dir.c_str();						// 初期フォルダー
	o.lpstrFile = fname;										// 取得したファイル名を保存するバッファ
	o.nMaxFile = sizeof(fname);									// 取得したファイル名を保存するバッファサイズ
	o.lpstrFilter = filters;									// (例) "TXTファイル(*.TXT)\0*.TXT\0全てのファイル(*.*)\0*.*\0";
	o.lpstrDefExt = defExt.data();
	o.lpstrTitle = title.data();
	o.nFilterIndex = 1;
	o.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	if (GetSaveFileNameA(&o))
	{
		// カレントディレクトリを元に戻す
		std::filesystem::current_path(current);
		// 相対パスへ変換
		filepath = std::filesystem::relative(fname).string();
		return true;
	}
	std::filesystem::current_path(current);	// カレントディレクトリを元に戻す
	return false;
}
