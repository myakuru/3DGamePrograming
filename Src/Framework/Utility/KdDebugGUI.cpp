#include"../../Application/main.h"
#include"../../MyFramework/Manager/ImGuiManager/ImGuiManager.h"

#include "KdDebugGUI.h"

KdDebugGUI::KdDebugGUI()
{}
KdDebugGUI::~KdDebugGUI()
{ 
	GuiRelease();
}

void KdDebugGUI::GuiInit()
{
	// 初期化済みなら動作させない
	if (m_uqLog) return;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	
	ImGuiIO& io = ImGui::GetIO();
	// ここの部分でImGuiのドッキング機能を有効にする
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		//	マルチウィンドウを有効にする
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		//	ドッキングを有効にする

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(Application::Instance().GetWindowHandle());
	ImGui_ImplDX11_Init(
		KdDirect3D::Instance().WorkDev(), KdDirect3D::Instance().WorkDevContext());

#include "imgui/ja_glyph_ranges.h"
	ImFontConfig config;
	config.MergeMode = true;
	//io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\SegoeIcons.ttf", 13.0f);
	// 日本語対応
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msgothic.ttc", 13.0f, &config, glyphRangesJapanese);

	m_uqLog = std::make_unique<ImGuiAppLog>();
}

void KdDebugGUI::GuiProcess()
{

	if (KeyboardManager::GetInstance().IsKeyJustPressed(VK_F2))
	{
		m_showImGui = !m_showImGui;
	}

	if (!m_showImGui) return;

	// 初期化されてないなら動作させない
	if (!m_uqLog) return;

	IdleBySleeping(params.fpsIdling);

	//===========================================================
	// ImGui開始
	//===========================================================
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// ===== DockSpaceの追加 =====
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		// 余白なしで全面DockSpace
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);

		// メインメニューバーを別で描く場合は MenuBar フラグ不要
		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		// ImGui推奨のオーバービューポート版を使用
		ImGui::Begin("DockSpaceHost", nullptr, window_flags);
		ImGuiID dockspace_id = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);
		ImGui::End();

		ImGui::PopStyleVar(3);
	}


	//===========================================================
	// 以下にImGui描画処理を記述
	//===========================================================

	// ログウィンドウ
	m_uqLog->Draw("Console");

	IMGUI_MANAGER.ImGuiUpdate(); // ImGuiManagerの更新処理を呼び出す
	

	//===========================================================
	// ここより上にImGuiの描画はする事
	//===========================================================
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// マルチウィンドウを有効にしている場合は、以下の処理も必要
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		// 描画、更新
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		// ここでレンダリングターゲットを戻す必要がある
		KdDirect3D::Instance().WorkDevContext()->OMSetRenderTargets(
			1,
			KdDirect3D::Instance().GetBackBuffer()->WorkRTViewAddress(),
			KdDirect3D::Instance().GetZBuffer()->WorkDSView());
	}
}

void KdDebugGUI::AddLog(const char* fmt,...)
{
	// 初期化されてないなら動作させない
	if (!m_uqLog) return;

	va_list args;
	va_start(args, fmt);
	m_uqLog->AddLog(fmt);
	va_end(args);
}

void KdDebugGUI::IdleBySleeping(FpsIdling& ioIdling)
{

	ioIdling.isIdling = false;
	if ((ioIdling.fpsIdle > 0.f) && ioIdling.enableIdling)
	{
		// スリープ前の時刻
		auto beforeWait = std::chrono::high_resolution_clock::now();
		double waitTimeout = 1.0 / ioIdling.fpsIdle;

		// イベント待ち＋タイムアウト
		WaitForEventTimeout(waitTimeout);

		// スリープ後の時刻
		auto afterWait = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> waitDuration = afterWait - beforeWait;
		double waitIdleExpected = 1.0 / ioIdling.fpsIdle;
		ioIdling.isIdling = (waitDuration.count() > waitIdleExpected * 0.9);
	}
}

void KdDebugGUI::GuiRelease()
{
	// 初期化されてないなら動作させない
	if (!m_uqLog) return;

	m_uqLog = nullptr;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void KdDebugGUI::WaitForEventTimeout(double timeoutSeconds)
{
	DWORD timeoutMs = static_cast<DWORD>(timeoutSeconds * 1000.0);
	// QS_ALLINPUT: すべての入力メッセージを対象
	MsgWaitForMultipleObjects(0, nullptr, FALSE, timeoutMs, QS_ALLINPUT);
}
